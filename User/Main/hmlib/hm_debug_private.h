/************************************************************************
 $Revision$                                                           
************************************************************************/

#ifndef __HM_DEBUG_PRIVATE_H
#define __HM_DEBUG_PRIVATE_H
#include "hm_types.h"
#include "hm_common_cfg.h"

/*find an empty buffer*/
hm_print_buf_t* hm_net_find_idle_buf(void);

/*push buffer to main loop*/
void hm_net_add_print_pending(hm_print_buf_t*);

bool hm_net_isdebugon(void);
#endif
