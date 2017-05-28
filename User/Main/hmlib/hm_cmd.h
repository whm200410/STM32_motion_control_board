#ifndef __HM_CMD_H
#define __HM_CMD_H
#include <stdint.h>
#include <hm_types.h>
void hm_cmd_init(void);
void hm_cmd_register(uint8_t ncmd, hm_ck_func_t ck);
#endif
