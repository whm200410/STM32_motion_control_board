/************************************************************************
 $Revision: 9 $                                                           
************************************************************************/
#ifndef __HM_MSG_QUEUE_H
#define __HM_MSG_QUEUE_H
#include "hm_types.h"
#include <stdint.h>

void	hm_msg_queue_init(void);
bool	hm_msg_queue_add(hm_ck_func_t	func, void *param);
void	hm_msg_queue_main(void);

#endif
