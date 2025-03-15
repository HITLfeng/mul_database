# coding=utf-8
from gm.api import *
import datetime
import pandas as pd
import numpy as np

TimeInForce_DAY = 0  # 当日有效
TimeInForce_GTC = 1  # 撤销前有效（已弃用）
TimeInForce_IOC = 2  # 立即成交剩余撤销
TimeInForce_FOK = 3  # 全部成交否则撤销

def init(context):
    context.config = {
        'symbols': ['SZSE.002172'],
        'total_position': 3000,
        'trade_volume': 100,
        'frequency': '60s',
        'history_minutes': 100,
        'macd_params': (12, 26, 9),
        'min_data_length': 30,
        'cooling_period': 300,  # 交易冷却时间（秒）
        'trade_hours': [
            (datetime.time(9, 30), datetime.time(11, 30)),
            (datetime.time(13, 0), datetime.time(15, 0))
        ]
    }

    context.position_status = {
        sym: {
            'established': False,
            'last_trade': None
        } for sym in context.config['symbols']
    }

    try:
        subscribe(
            symbols=context.config['symbols'],
            frequency=context.config['frequency'],
            count=context.config['history_minutes'],
            fields='symbol,eob,close,high,low,volume'
        )
    except Exception as e:
        log(f"数据订阅失败: {str(e)}")
        context.stop()


def on_bar(context, bars):
    current_time = context.now.time()
    symbol = bars[0]['symbol']

    # 交易时段验证
    if not is_trading_time(current_time, context.config['trade_hours']):
        return

    # 初始建仓
    if not context.position_status[symbol]['established']:
        initialize_position(context, symbol)
        return

    # 获取数据
    recent_data = get_validated_data(context, symbol)
    if recent_data is None:
        return

    # 涨跌停检查
    if check_limit_status(recent_data, symbol):
        return

    # MACD计算
    try:
        close_prices = recent_data['close'].values
        _, _, macd_line = MACD(close_prices, *context.config['macd_params'])
    except Exception as e:
        log(f"指标计算失败: {str(e)}")
        return

    # 交易冷却检查
    if should_skip_trading(context, symbol):
        return

    # 交易信号处理
    execute_trading_logic(context, symbol, macd_line)

    # 尾盘处理
    handle_closing_adjustment(context, symbol, current_time)


# ============== 核心函数 ==============
def initialize_position(context, symbol):
    """安全建立初始仓位"""
    try:
        current_price = safe_last_price(symbol)
        if current_price <= 0:
            raise ValueError("无效价格")

        order_target_volume(
            symbol=symbol,
            volume=context.config['total_position'],
            order_type=OrderType_Limit,
            position_side=PositionSide_Long,
            price=current_price
        )
        context.position_status[symbol]['established'] = True
        log(f"成功建立{symbol}底仓")
    except Exception as e:
        log(f"建仓失败: {str(e)}")


def get_validated_data(context, symbol):
    """带校验的数据获取"""
    try:
        data = history(
            symbol=symbol,
            frequency=context.config['frequency'],
            start_time=context.now - datetime.timedelta(
                minutes=context.config['history_minutes']
            ),
            end_time=context.now,
            fields='close,high,low,volume',
            adjust=ADJUST_PREV,
            df=True
        )

        # 数据校验
        if data.empty or len(data) < context.config['min_data_length']:
            log(f"数据不足，需要{context.config['min_data_length']}条，实际{len(data)}条")
            return None
        if data.isnull().values.any():
            log("存在空值数据")
            return None

        return data
    except Exception as e:
        log(f"数据获取错误: {str(e)}")
        return None


def check_limit_status(data, symbol):
    """涨跌停判断"""
    try:
        last_row = data.iloc[-1]
        is_limit = (last_row['high'] == last_row['low']) and (last_row['volume'] == 0)
        if is_limit:
            log(f"{symbol} 触发涨跌停")
        return is_limit
    except Exception as e:
        log(f"涨跌停检查异常: {str(e)}")
        return False


def should_skip_trading(context, symbol):
    """交易冷却检查"""
    last_trade = context.position_status[symbol]['last_trade']
    if last_trade and (context.now - last_trade).seconds < context.config['cooling_period']:
        log(f"{symbol} 处于交易冷却期")
        return True
    return False


def execute_trading_logic(context, symbol, macd_line):
    """执行交易信号"""
    if len(macd_line) < 2:
        log("MACD数据不足")
        return

    if macd_line[-2] <= 0 < macd_line[-1]:
        place_order(symbol, OrderSide_Buy, context.config['trade_volume'], context)
    elif macd_line[-2] >= 0 > macd_line[-1]:
        place_order(symbol, OrderSide_Sell, context.config['trade_volume'], context)


def handle_closing_adjustment(context, symbol, current_time):
    """尾盘仓位调整"""
    if datetime.time(14, 55) <= current_time <= datetime.time(15, 0):
        try:
            position = context.account().position(symbol, PositionSide_Long)
            if position and position.volume != context.config['total_position']:
                order_target_volume(
                    symbol=symbol,
                    volume=context.config['total_position'],
                    order_type=OrderType_Limit,
                    position_side=PositionSide_Long,
                    price=safe_last_price(symbol),
                    time_in_force=TimeInForce_IOC
                )
        except Exception as e:
            log(f"尾盘调仓失败: {str(e)}")


# ============== 工具函数 ==============
def safe_last_price(symbol):
    """安全获取最新价格"""
    try:
        return current(symbol)[0]['price']
    except Exception:
        try:
            return history(symbol=symbol, frequency='1d', count=1, fields='close')['close'].iloc[-1]
        except Exception as e:
            log(f"获取历史价格失败: {str(e)}")
            return 0


def is_trading_time(current_time, time_ranges):
    """验证交易时段"""
    return any(start <= current_time <= end for start, end in time_ranges)


def place_order(symbol, side, volume, context):
    """订单管理"""
    try:
        price = safe_last_price(symbol) * (1.01 if side == OrderSide_Buy else 0.99)
        order_volume(
            symbol=symbol,
            volume=volume,
            side=side,
            order_type=OrderType_Limit,
            price=round(price, 2),
            position_effect=PositionEffect_Close
            # time_in_force=TimeInForce_DAY
        )
        context.position_status[symbol]['last_trade'] = context.now
        log(f"{symbol} {OrderSide_Text[side]}订单提交")
    except Exception as e:
        log(f"订单失败: {str(e)}")


OrderSide_Text = {
    OrderSide_Buy: "买入",
    OrderSide_Sell: "卖出"
}


def log(msg):
    """日志记录"""
    print(f"[{datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}] {msg}")


# ============== 指标函数 ==============
def EMA(S: np.ndarray, N: int) -> np.ndarray:
    """指数移动平均"""
    return pd.Series(S).ewm(span=N, adjust=False).mean().values


def MACD(CLOSE: np.ndarray, SHORT=12, LONG=26, M=9):
    """MACD计算"""
    DIF = EMA(CLOSE, SHORT) - EMA(CLOSE, LONG)
    DEA = EMA(DIF, M)
    return DIF, DEA, (DIF - DEA) * 2


# ============== 回调函数 ==============
def on_order_status(context, order):
    """订单状态回调"""
    if order['status'] == 3:
        action = "买入" if order['side'] == 1 else "卖出"
        print(f"{order['symbol']} {action} {order['volume']}股 成交于 {order['price']}")


if __name__ == '__main__':
    run(
        strategy_id='17ae0685-f81e-11ef-aed2-107c61bc5c11',
        filename='main.py',
        mode=MODE_BACKTEST,
        token='47cffa2b963bb3966b2211333731024eae7d4b18',
        backtest_start_time='2025-03-01 09:00:00',
        backtest_end_time='2025-03-15 15:00:00',
        backtest_adjust=ADJUST_PREV,
        backtest_initial_cash=100000,
        backtest_commission_ratio=0.0003,
        backtest_slippage_ratio=0.001,
        backtest_match_mode=0
    )
