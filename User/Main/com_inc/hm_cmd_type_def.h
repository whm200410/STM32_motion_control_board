#ifndef __HM_CMD_TYPE_DEF_H
#define __HM_CMD_TYPE_DEF_H
#ifdef WIN32
#include "hm_types.h"
#else
#include <stdint.h>
#endif

#define HM_CMD_MAX_INPUT_SIZE       24
#define HM_CMD_MAX_OUTPUT_SIZE      8
#define HM_CMD_MARK1                0x98
#define HM_CMD_MARK2                0x76
#define HM_CMD_INVALID              0xFF
#define HM_CMD_OWER_CLIENT          9
#define HM_CMD_OWER_SERVER          8

#define UDP_SERVER_PORT             5123   /* define the UDP local connection port */
typedef struct hm_cmd_req_s
{   
    uint8_t     input[HM_CMD_MAX_INPUT_SIZE];
    uint8_t     input_len;
    uint8_t     ncmd;
    uint8_t     ret;
    uint8_t     mark1;
    uint8_t     mark2;
    uint8_t     owner;
}hm_cmd_req_t;


#endif
