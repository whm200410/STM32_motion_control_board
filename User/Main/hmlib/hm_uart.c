/************************************************************************
 $Revision: 44 $                                                           
************************************************************************/
#include "stm32f10x.h"
#include "hm_uart.h"
//#include "hm_uart_private.h"
#include "hm_msg_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
const uint32_t  HM_UART_MAX_CK_SIZE         = 20;


typedef struct hm_uart_ck_info_s
{
    hm_ck_func_t        func;
    uint8_t             ch;
}hm_uart_ck_info_t;



static hm_uart_ck_info_t    s_uart_cks[HM_UART_MAX_CK_SIZE];


static bool                 s_debug_off;

void hm_uart_debug_off(bool boff)
{
    s_debug_off  = boff;
}

/**
  * @brief  get if uart debug output is turn on/off 
  * @param  one
  * @retval true on, false off
  */
bool hm_uart_is_debug_off(void)
{
    return s_debug_off;
}



void hm_uart_init(void)
{
	
#ifdef HM_CMD_METHOD_UART 	    
    NVIC_InitTypeDef    NVIC_InitStructure;
#endif    
    USART_InitTypeDef   USART_InitStructure;
    GPIO_InitTypeDef    GPIO_InitStructure;
    uint32_t            i;

	
	for(i = 0; i < HM_UART_MAX_CK_SIZE; ++i)
	{
		s_uart_cks[i].func = 0;
	}   
     
    
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOD
						    |RCC_APB2Periph_AFIO, ENABLE);
    

#ifdef USE_PRINT
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);    
    /*USART1端口配置
	  PA9 TX 复用推挽输出 PA10 RX 浮空输入模式*/
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9 ;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 ;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
    
	USART_InitStructure.USART_BaudRate = 38400;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);    
#endif
    
#ifdef HM_CMD_METHOD_UART    
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	 

	/*USART2端口配置
	  PD5 TX 复用推挽输出 PD6 RX 浮空输入模式*/
	GPIO_PinRemapConfig(GPIO_Remap_USART2, ENABLE);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 ;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

    /*--------------USART1 配置-------------------------*/
	USART_InitStructure.USART_BaudRate = 38400;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);    
    USART_Cmd(USART2, ENABLE);
    
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);    
    
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; 		  //USART1接收中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //先占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  //次占优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);       
 
#endif    

    

}

#ifdef HM_CMD_METHOD_UART 
void USART2_IRQHandler(void)
{
	uint8_t     temp1;
    uint32_t    i;
	if(USART_GetITStatus(USART2, USART_IT_RXNE)==SET)
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		temp1=USART_ReceiveData(USART2);
        
        for(i = 0; i < HM_UART_MAX_CK_SIZE; ++i)
        {
            if(s_uart_cks[i].func)
            {
                s_uart_cks[i].ch = temp1;
                s_uart_cks[i].func(&s_uart_cks[i].ch);
            }
        }
	}
}
#endif

#ifdef USE_PRINT  
int fputc(int ch, FILE *f)
{
 
  USART_SendData(USART1, (uint8_t) ch);   					   //发送一个字符

  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)  //等待发送完成
  {}

  return ch;
}
#endif

void hm_uart_send_cmd(uint8_t *pbuf, uint32_t len)
{
    uint32_t idx;
    for(idx = 0; idx < len; ++idx)
    {
        USART_SendData(USART2,  pbuf[idx]);   					   //发送一个字符
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    }
}
void hm_uart_send_log(uint8_t *pbuf, uint32_t len)
{
    uint32_t idx;
    for(idx = 0; idx < len; ++idx)
    {
        USART_SendData(USART1,  pbuf[idx]);   					   //发送一个字符
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }
}
hm_uart_handler_t hm_reg_uart_rev_ck(hm_ck_func_t func)
{
	uint32_t            i;
    hm_uart_ck_info_t   *pret = NULL;
	if(func == 0) return NULL;
		
	for(i = 0; i < HM_UART_MAX_CK_SIZE; ++i)
	{
        if(s_uart_cks[i].func == func) return NULL;
		if(s_uart_cks[i].func == NULL && !pret)
		{
            pret = &s_uart_cks[i];
		}
	}   
    if(pret)
    {
        pret->func      = func;   
    }
    return pret;    
}
void hm_unreg_uart_rev_ck(hm_uart_handler_t par)
{
    hm_uart_ck_info_t   *pret = (hm_uart_ck_info_t   *)par;
    pret->func = 0;
}
