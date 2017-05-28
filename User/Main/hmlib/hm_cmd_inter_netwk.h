/************************************************************************
 $Revision: 44 $                                                           
************************************************************************/

#ifndef __HM_CMD_INTER_NETWK_H
#define __HM_CMD_INTER_NETWK_H
#include <stdint.h>
#include <hm_types.h>

/*network ini and register callback function from message cmd*/
void hm_cmd_net_init(void);
void hm_cmd_net_register(uint8_t ncmd, hm_ck_func_t ck);

/*for debug purpose*/
void hm_cmd_net_init_debug(bool bdebug_on, uint16_t nport);


#endif
