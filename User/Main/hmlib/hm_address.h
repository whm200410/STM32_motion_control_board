/************************************************************************
 $Revision$                                                           
************************************************************************/
#ifndef __HM_ADDRESS_H
#define __HM_ADDRESS_H
#include "hm_types.h"
#include <stdint.h>
bool     hm_addr_init(void);
uint8_t* hm_mac_addr(void);
uint8_t* hm_ip_addr(void);

#endif
