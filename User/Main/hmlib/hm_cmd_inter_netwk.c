/************************************************************************
 $Revision: 46 $                                                           
************************************************************************/
#include "stm32f10x.h"
#include "hm_cmd_type_def.h"
#include "hm_cmd_inter_netwk.h"
#include "hm_msg_queue.h"
#include "hm_common_cfg.h"
#include "hw_config.h"
#include "hm_timer.h"
#include <string.h>
#include "hm_debug_private.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include <stdio.h>

#define HM_CMD_MAX_CMD_ITEM     20
#define HM_CMD_MAX_CMD_MAP      20
#define HM_CMD_INVALID_IDX      0xFFFF
#define HM_UART_PRINT_BUF_ITEM_SZ 10
//used by this 

//every cmd queue item's stage
typedef enum 
{
    hmCmdStIdle,
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
    hm_cmd_req_t        cmd_req;
    HM_CMD_ITEM_STATE_E state;
    struct pbuf         *pbuf;
    struct udp_pcb      *upcb;
}hm_cmd_item_t;

typedef struct hm_debug_s
{
    uint32_t            bdebug_on;
    struct pbuf         *pbuf;
    struct udp_pcb      *upcb;
    uint16_t            nport;
    struct ip_addr      paddr;
    hm_print_buf_t      pt_buf;
}hm_debug_t;

//cmd queue and cmd table
static  hm_cmd_item_t       s_cmd_queue[HM_CMD_MAX_CMD_ITEM];
static  hm_cmd_ck_pair_t    s_cmd_table[HM_CMD_MAX_CMD_MAP];



static  hm_debug_t          s_debug;
//static  hm_print_buf_t      s_uart_prints[HM_UART_PRINT_BUF_ITEM_SZ];
//cmd buf current using
static  uint32_t            s_queue_idx;

/*find an idle cmd buf from queue*/
static hm_cmd_item_t*       hm_cmd_find_idle(uint32_t *pidx);

/*find a empty spot for cmd map*/
static hm_cmd_ck_pair_t*    hm_cmd_find_cmd_table(uint8_t ncmd);


/*init */
static void hm_cmd_queue_item_init(hm_cmd_item_t *);
static void hm_cmd_item_init(hm_cmd_req_t *);

/*used to called in main, call the responding ck of ncmd*/
static void hm_cmd_msg_ck(void*);

static void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);

static void* bytescopy(void *des, void *src, uint32_t len);

static void hm_net_debug_send(uint8_t *pbuf, uint32_t len);
void hm_cmd_net_init(void)
{
    int idx ;
 
    
    struct udp_pcb *upcb;                                 

    /* Create a new UDP control block  */
    upcb = udp_new();

    /* Bind the upcb to the UDP_PORT port */
    /* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
    udp_bind(upcb, IP_ADDR_ANY, HM_UDP_PORT);

    /* Set a receive callback for the upcb */
    udp_recv(upcb, udp_server_callback, NULL);

    for(idx = 0; idx < HM_CMD_MAX_CMD_ITEM; ++idx)
    {
        hm_cmd_queue_item_init(&s_cmd_queue[idx]);
    }
    for(idx = 0; idx < HM_CMD_MAX_CMD_MAP; ++idx)
    {
        s_cmd_table[idx].ck     = 0;
        s_cmd_table[idx].ncmd   = HM_CMD_INVALID;
    }   
    s_debug.bdebug_on   = false;
    s_debug.nport       = 0;
#if 0    
 	for(idx = 0;idx < HM_UART_PRINT_BUF_ITEM_SZ; ++idx)
	{
		s_uart_prints[idx].bvalid = false;
	}    
#endif    
}
void hm_cmd_net_init_debug(bool bdebug_on, uint16_t nport)
{
    s_debug.bdebug_on   = bdebug_on;
    if(bdebug_on)
    {
        if(!s_debug.upcb)
        {
            s_debug.pbuf        = pbuf_alloc(PBUF_TRANSPORT,sizeof(hm_print_buf_t) + 20, PBUF_POOL);
            s_debug.upcb        = udp_new();
            udp_bind(s_debug.upcb, IP_ADDR_ANY, HM_UDP_PORT_LOG);
        }

        s_debug.nport   = nport;
        udp_connect(s_debug.upcb, &s_debug.paddr, nport);
    }
    else
    {
        pbuf_free(s_debug.pbuf);
        udp_remove(s_debug.upcb);
        s_debug.pbuf    = 0;
        s_debug.upcb    = 0;
    }
}
void hm_net_debug_send(uint8_t *pbuf, uint32_t len)
{
    if(s_debug.bdebug_on && s_debug.pbuf && s_debug.upcb)
    {
        pbuf_take(s_debug.pbuf, pbuf, len);
        udp_send(s_debug.upcb, s_debug.pbuf);
    }
}
bool hm_net_isdebugon(void)
{
    return s_debug.bdebug_on;
}
/**
  * @brief This function is called when an UDP datagrm has been received on the port UDP_PORT.
  * @param arg user supplied argument (udp_pcb.recv_arg)
  * @param pcb the udp_pcb which received data
  * @param p the packet buffer that was received
  * @param addr the remote IP address from which the packet was received
  * @param port the remote port from which the packet was received
  * @retval None
  */
void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{                          	     
    
    hm_cmd_item_t   *pcmd_item  = NULL;
    hm_cmd_req_t    *preq       = NULL;
    bool            bfree       = false;
    /* Connect to the remote client */
    udp_connect(upcb, addr, port);

    /*if the length is not right, then send back ret = false*/

    if(p->len != sizeof(hm_cmd_req_t)) bfree = true;

    /*check the mark*/
    preq    = p->payload;
    if(!bfree && (preq->mark1 != HM_CMD_MARK1 || preq->mark2 != HM_CMD_MARK2 || preq->owner != HM_CMD_OWER_CLIENT)) bfree = true;
    
    
    /*if we can alloc free buf*/
    if(bfree || (pcmd_item = hm_cmd_find_idle(&s_queue_idx)) == NULL)
    {
        udp_send(upcb, p);
        udp_disconnect(upcb);
        pbuf_free(p);
        return;
    }
    
    /*now record and fill buf*/
    pcmd_item->pbuf     = p;
    pcmd_item->upcb     = upcb;    
    pcmd_item->state    = hmCmdStPushed;
    bytescopy(&pcmd_item->cmd_req, preq, sizeof(hm_cmd_req_t));
    
    s_debug.paddr = *addr;
    hm_cmd_msg_ck(pcmd_item);
    
//    if(!hm_msg_queue_add(hm_cmd_msg_ck, pcmd_item))
 //   {
        
 //   }
}



void hm_cmd_net_register(uint8_t ncmd, hm_ck_func_t ck)
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
    pinfo->state            = hmCmdStIdle;  
    pinfo->pbuf             = 0;
    pinfo->upcb             = 0;   
    hm_cmd_item_init(&pinfo->cmd_req);
    
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
    hm_cmd_ck_pair_t    *pcmd_ck    = hm_cmd_find_cmd_table(pcmd_item->cmd_req.ncmd);
    
    //check error, this should not be happen
    if(!pcmd_ck || !pcmd_ck->ck) return;
    
    //set stage, call it , then init buffer
    pcmd_item->state = hmCmdStCalling;
    
    pcmd_ck->ck(&pcmd_item->cmd_req);
    
    
    bytescopy(pcmd_item->pbuf->payload, &pcmd_item->cmd_req, sizeof(hm_cmd_req_t));
    udp_send(pcmd_item->upcb, pcmd_item->pbuf);
    udp_disconnect(pcmd_item->upcb);
    
    /* Free the p buffer */
    pbuf_free(pcmd_item->pbuf);
    hm_cmd_queue_item_init(pcmd_item);

}
void* bytescopy(void *des, void *src, uint32_t num)
{
    uint8_t * psrc = (uint8_t *)src;//byte ??unsigned char??  
    uint8_t * pdst = (uint8_t *)des;  
    while(num-->0)*pdst++ = *psrc++;  
    return des;  
}
void hm_net_print_ck(void* param)
{
    hm_print_buf_t *pinf = (hm_print_buf_t*)param;
    
    hm_net_debug_send((uint8_t*)pinf->sbuf, strlen(pinf->sbuf));
}
hm_print_buf_t* hm_net_find_idle_buf(void)
{
    memset(&s_debug.pt_buf.sbuf, 0, HM_UART_PRINT_BUF_SIZE);
    return &s_debug.pt_buf;
}
void hm_net_add_print_pending(hm_print_buf_t* ppnt_buf)
{
    hm_net_debug_send((uint8_t*)ppnt_buf, sizeof(hm_print_buf_t));
}
