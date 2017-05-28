/************************************************************************
 $Revision: 44 $                                                           
************************************************************************/

#ifndef __HM_TIMER_H
#define __HM_TIMER_H
#include <stdint.h>
#include "hm_types.h"

typedef void*   hm_timer_handler_t;
typedef uint32_t    tim_count_t;
#define HM_TIM_INVALID_COUNT    0xffff
/*need to init before use*/
void hm_timer_init(void);

/*should not called in interrupt*/
void hm_delay(tim_count_t ms);

/*calculate tim diff between 2 tim count, ms*/
tim_count_t    hm_tim_diff(tim_count_t tm_earliar, tim_count_t tm_later);

/*get current tim ms*/
tim_count_t hm_get_cur_time(void);


/*register timer, once means if just call once*/
hm_timer_handler_t hm_timer_register(tim_count_t ms, hm_ck_func_t func, void *param, bool once, bool bcall_direct);

/*unregister timer*/
void        hm_timer_unregister(hm_timer_handler_t);

#endif
