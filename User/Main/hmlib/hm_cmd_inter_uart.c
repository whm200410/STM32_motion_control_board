/************************************************************************
 $Revision: 44 $                                                           
************************************************************************/
#include "stm32f10x.h"
#include "hm_uart.h"
#include "hm_cmd_type_def.h"
#include "hm_cmd_inter_uart.h"
#include "hm_msg_queue.h"
#include "hw_config.h"
#include "hm_led.h"
#include "hm_timer.h"
#include "hm_debug.h"
#include <stdio.h>
#define HM_CMD_MAX_CMD_ITEM     20
#define HM_CMD_MAX_CMD_MAP      20
#define HM_CMD_INVALID_IDX      0xFFFF
//used by this 

//every cmd queue item's stage
typedef enum 
{
    hmCmdStIdle,
    hmCmdStFilling,
    hmCmdStPushed,
    hmCmdStCalling,
}HM_CMD_ITEM_STATE_E;

//cmd table, ncmd conresponding to callback
typedef struct hm_cmd_ck_pair_s
{
    uint8_t         ncmd;
    hm_ck_func_t    ck;
}hm_cmd_ck_pair_t;


//cmd queue, record state, input buff
typedef struct hm_cmd_item_s
{
    uint32_t             buf_idx;
    hm_cmd_req_t        *pcmd;
    uint8_t             cmd_buf[sizeof(hm_cmd_req_t)];
    HM_CMD_ITEM_STATE_E state;
}hm_cmd_item_t;


//cmd queue and cmd table
static  hm_cmd_item_t       s_cmd_queue[HM_CMD_MAX_CMD_ITEM];
static  hm_cmd_ck_pair_t    s_cmd_table[HM_CMD_MAX_CMD_MAP];

//cmd buf current using
static  uint32_t            s_queue_idx;

/*find an idle cmd buf from queue*/
static hm_cmd_item_t*       hm_cmd_find_idle(uint32_t *pidx);

/*find a empty spot for cmd map*/
static hm_cmd_ck_pair_t*    hm_cmd_find_cmd_table(uint8_t ncmd);

/*call back of uart cmd ,used to fill cmd buf*/
static void hm_cmd_uart_ck(void* param);

/*init */
static void hm_cmd_queue_item_init(hm_cmd_item_t *);
static void hm_cmd_item_init(hm_cmd_req_t *);

/*used to called in main, call the responding ck of ncmd*/
static void hm_cmd_msg_ck(void*);


void hm_cmd_init(void)
{
    uint32_t idx;
    for(idx = 0; idx < HM_CMD_MAX_CMD_ITEM; ++idx)
    {
        hm_cmd_queue_item_init(&s_cmd_queue[idx]);
    }
    for(idx = 0; idx < HM_CMD_MAX_CMD_MAP; ++idx)
    {
        s_cmd_table[idx].ck     = 0;
        s_cmd_table[idx].ncmd   = HM_CMD_INVALID;
    }   
    
    //register the input path callback
    hm_reg_uart_rev_ck(hm_cmd_uart_ck);
    s_queue_idx = HM_CMD_INVALID_IDX;
}


static uint32_t s_cks = 0;
void hm_cmd_uart_ck(void* param)
{
    uint8_t         *pch        = (uint8_t *)param;
    hm_cmd_item_t   *pcmd_item  = NULL;
    static tim_count_t sl_last_tim = HM_TIM_INVALID_COUNT;
    s_cks++;
    //if current cmd buff is invalid or waiting for be called in main , find a idle one
    if(s_queue_idx == HM_CMD_INVALID_IDX || s_cmd_queue[s_queue_idx].state != hmCmdStFilling)
    {
        if((pcmd_item = hm_cmd_find_idle(&s_queue_idx)) == NULL)    //something wrong , maybe cmd buff to small or host send too many
        {
            hm_led_act(ltError, true);
            return;
        }
        
        pcmd_item->state = hmCmdStFilling;  // state is filling the buffer
    }
    else 
    {
        pcmd_item = &s_cmd_queue[s_queue_idx];
    }
    
    if(sl_last_tim != HM_TIM_INVALID_COUNT && hm_tim_diff(sl_last_tim, hm_get_cur_time()) > 300)
    {
        HM_NET_PRINT ("reset cmd :%d %d, %d\n", s_cks, pcmd_item->buf_idx, 
            pcmd_item->buf_idx > 0 ? pcmd_item->cmd_buf[pcmd_item->buf_idx- 1] : pcmd_item->cmd_buf[0]);
        pcmd_item->buf_idx = 0;
        pcmd_item->state = hmCmdStFilling;
    }
    
    
    pcmd_item->cmd_buf[pcmd_item->buf_idx++] = *pch;

    //if get enough buffer
    if(pcmd_item->buf_idx == sizeof(hm_cmd_req_t))
    {
        HM_NET_PRINT( "cmd transfer done %d \r\n", s_cks);    
        //first check for error
        if(pcmd_item->pcmd->mark1 != HM_CMD_MARK1 || pcmd_item->pcmd->mark2 != HM_CMD_MARK2)
        {
            hm_led_act(ltError, true);
            HM_NET_PRINT( "cmd transfer error 1  %d \r\n", s_cks);           
            hm_cmd_queue_item_init(pcmd_item);
        }
        else
        {
            //set state and add to main queue, make sure that the cmd call must called one by one ,in one thread

            HM_NET_PRINT( "cmd transfer push %d \r\n", s_cks); 
            pcmd_item->state = hmCmdStPushed;
            hm_msg_queue_add(hm_cmd_msg_ck, pcmd_item);   
        }
    }
    sl_last_tim = hm_get_cur_time();
    
}

void hm_cmd_register(uint8_t ncmd, hm_ck_func_t ck)
{
    uint32_t idx;
    for(idx = 0; idx < HM_CMD_MAX_CMD_MAP; ++idx)
    {
        if(s_cmd_table[idx].ncmd   == HM_CMD_INVALID)
        {
            s_cmd_table[idx].ncmd = ncmd;
            s_cmd_table[idx].ck = ck;
            break;
        }
    }    
}
hm_cmd_item_t*   hm_cmd_find_idle(uint32_t *pidx)
{
    uint32_t idx;
    for(idx = 0; idx < HM_CMD_MAX_CMD_ITEM; ++idx)
    {
        if(s_cmd_queue[idx].state == hmCmdStIdle) 
        {
            if(pidx) *pidx = idx;
            return &s_cmd_queue[idx];
        }
    }    
    
    return NULL;
}
hm_cmd_ck_pair_t*    hm_cmd_find_cmd_table(uint8_t ncmd)
{
    uint32_t idx;
    for(idx = 0; idx < HM_CMD_MAX_CMD_ITEM; ++idx)
    {
        if(s_cmd_table[idx].ncmd == ncmd) return &s_cmd_table[idx];
    }    
    
    return NULL;    
}
void hm_cmd_queue_item_init(hm_cmd_item_t *pinfo)
{
    pinfo->buf_idx          = 0;
    pinfo->pcmd             = (hm_cmd_req_t*)pinfo->cmd_buf;
    pinfo->state            = hmCmdStIdle;       
    hm_cmd_item_init(pinfo->pcmd);
    
}
void hm_cmd_item_init(hm_cmd_req_t * pcmd)
{
    pcmd->ncmd              = HM_CMD_INVALID;
    pcmd->input_len         = 0;
    pcmd->mark1             = 0;
    pcmd->mark2             = 0;
}
void hm_cmd_msg_ck(void* param)
{
    hm_cmd_item_t       *pcmd_item  = (hm_cmd_item_t   *)param;
    hm_cmd_ck_pair_t    *pcmd_ck    = hm_cmd_find_cmd_table(pcmd_item->pcmd->ncmd);
    
    //check error, this should not be happen
    if(!pcmd_ck || !pcmd_ck->ck) return;
    
    //set stage, call it , then init buffer
    pcmd_item->state = hmCmdStCalling;
    
    pcmd_ck->ck(pcmd_item->pcmd);
    
 
    hm_uart_send_cmd(pcmd_item->cmd_buf, sizeof(hm_cmd_req_t));

    
    hm_cmd_queue_item_init(pcmd_item);
}
