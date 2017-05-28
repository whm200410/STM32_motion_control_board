/************************************************************************
 $Revision: 44 $                                                           
************************************************************************/

#include "hm_msg_queue.h"
#include "hm_led.h"
const uint32_t HM_MSG_MAX = 80;
/*****************************************************************************
*
* STRUCTS
*
*****************************************************************************/
typedef struct msg_queue_s
{
	hm_ck_func_t	    func;
	void		        *param;
    bool                busing;
}msg_queue_t,*p_msg_queue_t;


typedef struct msg_queues_t
{
    msg_queue_t	    msg_array[HM_MSG_MAX];
    uint32_t        nwrite;
    uint32_t        nread;
    uint32_t        nmin;
}msg_queues_t, *p_msg_queues_t;

/*****************************************************************************
*
* VARIABLES
*
*****************************************************************************/
/*the variable store all message to be process*/
static msg_queues_t	s_msgs;
static bool ismsg_full(p_msg_queues_t pmsgs);
static bool ismsg_empty(p_msg_queues_t pmsgs);
static uint32_t msg_remaining(p_msg_queues_t pmsgs);
static p_msg_queue_t msg_get_move_next_write(p_msg_queues_t pmsgs);
static p_msg_queue_t msg_get_read(p_msg_queues_t pmsgs);
static void msg_move_next_read(p_msg_queues_t pmsgs, p_msg_queue_t pmsg);

/*****************************************************************************
 * msg_queue_init                                                        *
 ****************************************************************************/
/** Initialization function for msg queue
 *
 * @param No parameters.
 *
 * @returns No return value.
 *
 ****************************************************************************/

void	hm_msg_queue_init()
{
	uint32_t i;
	for(i = 0; i < HM_MSG_MAX; ++i)
	{
		s_msgs.msg_array[i].func    = 0;
        s_msgs.msg_array[i].busing  = 0;
	}
    s_msgs.nwrite = s_msgs.nread = 0;
    s_msgs.nmin = HM_MSG_MAX;
}
/*****************************************************************************
 * msg_queue_add                                                        *
 ****************************************************************************/
/** add a event to msg queue
 *
* @param func: function will be called, 
* @param param: parameter of function
 *
 * @returns bool return success or not.
 *
 ****************************************************************************/

bool	hm_msg_queue_add(hm_ck_func_t	func, void *param)
{
    p_msg_queue_t   pmsg = NULL;
	if(func == 0 || (pmsg = msg_get_move_next_write(&s_msgs)) == NULL) 
    {
        hm_led_act(ltError, true);
        return false;
    }
		
    if(s_msgs.nmin > msg_remaining(&s_msgs)) s_msgs.nmin = msg_remaining(&s_msgs);
    pmsg->func  = func;
    pmsg->param = param;
	return true;
}
uint32_t hm_msg_queue_nmin_msg(void)
{
    return s_msgs.nmin;
}
/*****************************************************************************
 * msg_queue_main                                                        *
 ****************************************************************************/
/** this function will be called in main loop, it handles all events stored
 *
* @param none
 *
 * @returns No return value.
 *
 ****************************************************************************/
void	hm_msg_queue_main()
{
    p_msg_queue_t   pmsg = NULL;
    if((pmsg = msg_get_read(&s_msgs)) == NULL) return;
    
    pmsg->func(pmsg->param);
    msg_move_next_read(&s_msgs, pmsg);
}

bool ismsg_full(p_msg_queues_t pmsgs)
{
    return ((pmsgs->nwrite + 1) % HM_MSG_MAX == pmsgs->nread);
}
uint32_t msg_remaining(p_msg_queues_t pmsgs)
{
    uint32_t nremaining = 0;
    nremaining = (pmsgs->nwrite < pmsgs->nread) ? (pmsgs->nread - pmsgs->nwrite) : (HM_MSG_MAX - pmsgs->nwrite + pmsgs->nread);
    return nremaining;
}
bool ismsg_empty(p_msg_queues_t pmsgs)
{
    return (pmsgs->nwrite == pmsgs->nread);
}
p_msg_queue_t msg_get_move_next_write(p_msg_queues_t pmsgs)
{
    p_msg_queue_t pret = NULL;
    if(ismsg_full(pmsgs)) return pret;
    
    __disable_irq();    
    pret = &pmsgs->msg_array[pmsgs->nwrite]; 
    pret->busing = true;    
    pmsgs->nwrite = (pmsgs->nwrite + 1) % HM_MSG_MAX;
    __enable_irq();
    return pret;
}
p_msg_queue_t msg_get_read(p_msg_queues_t pmsgs)
{
    p_msg_queue_t pret = NULL;
    if(!ismsg_empty(pmsgs)) pret = &pmsgs->msg_array[pmsgs->nread];        
    return pret;    
}
void msg_move_next_read(p_msg_queues_t pmsgs, p_msg_queue_t pmsg)
{
    if(pmsg != &pmsgs->msg_array[pmsgs->nread])
    {
        hm_led_act(ltError, true);
        while(1);
    }    
    __disable_irq();
    pmsg->busing = false;
    pmsgs->nread = (pmsgs->nread + 1) % HM_MSG_MAX;
    __enable_irq();    
}
