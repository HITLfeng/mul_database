#include "main_worker.h"
#include "seri_utils.h"
#include "outfunction.h"

Status RtHandleAddTest(char *usrMsg, char *resultBuf, uint32_t bufLen)
{
    // 序列化格式 int32_t int32_t char
    uint8_t **bufCursor = (uint8_t **)&usrMsg;
    int32_t argL = DeseriIntM(bufCursor);
    int32_t argR = DeseriIntM(bufCursor);
    CalcOptionT argOp = DeseriCharM(bufCursor);
    switch (argOp)
    {
    case CALC_ADD:
        SeriInt((uint8_t **)&resultBuf, argL + argR);
        break;
    case CALC_SUB:
        SeriInt((uint8_t **)&resultBuf, argL - argR);
        break;
    case CALC_MUL:
        SeriInt((uint8_t **)&resultBuf, argL * argR);
        break;
    case CALC_DIV:
        if (argR == 0)
        {
            error_info("add test invaild option, option is \\ and argR is 0.");
            return GMERR_ADD_TEST_INVAILD_OPTION;
        }
        SeriInt((uint8_t **)&resultBuf, argL / argR);
        break;
    default:
        error_info("add test invaild option, option is %c", argOp);
        return GMERR_ADD_TEST_INVAILD_OPTION;
    }
    return GMERR_OK;
}

Status RTProcessOpcode(OperatorCode opCode, char *usrMsg, char *resultBuf, uint32_t bufLen)
{
    switch (opCode)
    {
    case OP_ADD_TEST:
        return RtHandleAddTest(usrMsg, resultBuf, bufLen);
    default:
        break;
    }
    return GMERR_OK;
}