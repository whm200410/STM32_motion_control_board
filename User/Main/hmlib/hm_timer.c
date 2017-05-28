/************************************************************************
 $Revision: 49 $                                                           
************************************************************************/
#include "stm32f10x.h"
#include "hm_timer.h"
#include "hm_msg_queue.h"

const uint32_t HM_MAX_TIMER   = 10;
typedef struct hm_timer_s
{
	hm_ck_func_t    func;
	void		    *param;
	tim_count_t	    period;
	tim_count_t     ms;
    bool            once;
    bool            bcall_direct;
}hm_timer_t;

static hm_timer_t s_timers[HM_MAX_TIMER];
static volatile  tim_count_t   s_tim_count;
void hm_timer_init(void)
{
    uint32_t idx;
    for(idx = 0; idx < HM_MAX_TIMER; ++idx)
    {
        s_timers[idx].func  = 0;
    }
    s_tim_count = 0;
    
    /* Setup SysTick Timer for 1 msec interrupts  */
    if (SysTick_Config(SystemCoreClock / 1000))
    { 
        /* Capture error */ 
        while (1);
    }
    /* Set SysTick Priority to 3 */
    NVIC_SetPriority(SysTick_IRQn, 0x0C);
}

void hm_delay(tim_count_t ms)
{
	tim_count_t despire = (s_tim_count + ms);
	while(s_tim_count > despire);
	while(s_tim_count <= despire);
}

hm_timer_handler_t hm_timer_register(tim_count_t ms, hm_ck_func_t func, void *param, bool once, bool bcall_direct)
{
    uint32_t    idx;
    hm_timer_t  *pret = 0;
    for(idx = 0; idx < HM_MAX_TIMER; ++idx)
    {
        if(s_timers[idx].func  == 0 && pret == 0)
        {
            pret = &s_timers[idx];
        }
        if(s_timers[idx].func == func && s_timers[idx].ms == ms)
        {
            return NULL;
        }
    }
    if(pret)
    {
        pret->func  = func;
        pret->ms    = pret->period = ms ;
        pret->param = param;
        pret->once  = once;
        pret->bcall_direct  = bcall_direct;
    }
    return pret;
}

void hm_timer_unregister(hm_timer_handler_t hand)
{
    hm_timer_t  *ptm = (hm_timer_t  *)hand;
    if(ptm) ptm->func = 0;
    
}
tim_count_t    hm_tim_diff(tim_count_t tm_earliar, tim_count_t tm_later)
{
    if(tm_later >= tm_earliar)
    {
        return tm_later - tm_earliar;
    }
    else
    {
        return ((tim_count_t)(-1) - tm_earliar) + tm_later;
    }
}
tim_count_t hm_get_cur_time(void)
{
    return s_tim_count;
}
void SysTick_Handler(void)
{
    uint32_t    idx;
    s_tim_count++;
 //   s_tim_count = s_tim_count_inter / 10;
    for(idx = 0; idx < HM_MAX_TIMER; ++idx)
    {
        if(s_timers[idx].func == NULL) continue;
        s_timers[idx].ms--;
        if(s_timers[idx].ms == 0)
        {
            s_timers[idx].ms = s_timers[idx].period;
            if(s_timers[idx].bcall_direct)
                s_timers[idx].func(s_timers[idx].param);
            else
                hm_msg_queue_add(s_timers[idx].func, s_timers[idx].param);
            if(s_timers[idx].once) s_timers[idx].func = NULL;
            break;
        }
    }    
}
