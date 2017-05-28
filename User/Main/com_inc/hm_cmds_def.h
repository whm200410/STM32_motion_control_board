#ifndef __HM_CMDS_DEF_H
#define __HM_CMDS_DEF_H
#ifdef WIN32
#include "hm_types.h"
#else
#include <stdint.h>
#endif

#include "hm_struct_public.h"
typedef enum
{
    hmCmdFixMove,
    hmCmdKeepMove,
    hmCmdStopMove,
    hmCmdEnableAxis,
    hmCmdGetAxisStatus,
    hmCmdGetAxisSetting,
    hmCmdCmdLed,
    hmCmdSetPos,
    hmCmdGetSystemInfo,
    hmCmdIo,
    hmGetVersion,
    hmResetBoard,
    hmDebugOpe,
		hmResetForLoad,
    hmCmdMax
}HM_CMDS_ENUM;


typedef struct hmCmdFixMove_s
{
    uint32_t axis;
    int32_t  step;
    uint32_t accel;
    uint32_t decel;
    int32_t  speed;
}hmCmdFixMove_t;

typedef struct hmCmdKeepMove_s
{
    uint32_t axis;
    uint32_t accel;
    uint32_t decel;
    int32_t speed;
    uint32_t bhome;
}hmCmdKeepMove_t;

typedef struct hmCmdStopMove_s
{
    uint32_t axis;
    uint8_t  bsmooth;
}hmCmdStopMove_t;
typedef struct hmCmdEnableAxis_s
{
    uint32_t axis;
    uint32_t benable;
}hmCmdEnableAxis_t;

typedef struct hmCmdCmdLed_s
{
    uint32_t    idx;
    uint32_t    bon;
}hmCmdCmdLed_t;

typedef struct hmCmdSetPos_s
{
    uint32_t    axis;
    int32_t     pos;
}hmCmdSetPos_t;

typedef struct hmCmdIo_s
{
    uint32_t    idx;
    uint32_t    bread;
    uint32_t    bout;
    uint32_t    val_set;
    uint32_t    val_ret;
}hmCmdIo_t;

typedef struct hmCmdGetSystemInfo_s
{
    uint32_t    bdebug_off;
    uint32_t    bset;
    int16_t     ntemperature; 
    uint8_t     naxis_nm;
    uint8_t     ninput_nm;
    uint8_t     nout_nm;
}hmCmdGetSystemInfo_t;

typedef struct hmGetVersion_s
{
    uint16_t    nversion;
}hmGetVersion_t;

typedef struct hmDebugOpe_s
{
    uint32_t    bdebug_on;
    uint16_t    nport;
}hmDebugOpe_t;



#endif
