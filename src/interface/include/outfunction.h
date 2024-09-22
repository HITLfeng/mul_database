#include "out_type_defs.h"

typedef uint32_t CliStatus;

typedef enum CalcOption {
    CALC_ADD = 0,
    CALC_SUB,
    CALC_MUL,
    CALC_DIV,
    CALC_BUTT
} CalcOptionT;

#ifdef __cplusplus
extern "C"
{
#endif


// 启动服务
CliStatus KVCSrvStart(void);
CliStatus KVCSrvStop(void);

// 建联 断联函数

CliStatus KVCConnect(KVConnectT *conn);
CliStatus KVCDisconnect(KVConnectT *conn);

// 收发报文函数
CliStatus KVCSend(KVConnectT *conn, const MsgBufRequestT *msgBuf);
CliStatus KVCRecv(KVConnectT *conn, MsgBufResponseT *msgBuf);

// 对外接口测试函数
CliStatus KVCCalcTwoNumber(KVConnectT *conn, int x, int y, CalcOptionT opt, UsrResultBaseT *result);





#ifdef __cplusplus
}
#endif