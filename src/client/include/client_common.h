#ifndef __CLIENT_COMMON_H__
#define __CLIENT_COMMON_H__


#include "../../interface/include/out_type_defs.h"
#include "../../common/include/seri_utils.h"
#ifdef __cplusplus
extern "C"
{
#endif

uint8_t *GetUsrDataPosition(uint8_t *usrMsgBuf);

#ifdef __cplusplus
}
#endif

#endif // __CLIENT_COMMON_H__

