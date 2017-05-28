/**
  ******************************************************************************
  * @file    server.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    11/20/2009
  * @brief   A sample UDP/TCP server application.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2009 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include <string.h>
#include <stdio.h>
#include "Gpio_IO.h"

#include "stm32f10x.h"
#include "netconf.h"
#include "lcd.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define UDP_SERVER_PORT    5123   /* define the UDP local connection port */
#define UDP_CLIENT_PORT    4   /* define the UDP remote connection port */
#define TCP_PORT    4	/* define the TCP connection port */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void udp_server_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
err_t tcp_server_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the server application.
  * @param  None
  * @retval None
  */
void server_init(void)
{
   struct udp_pcb *upcb;                                 
   
   /* Create a new UDP control block  */
   upcb = udp_new();
   
   /* Bind the upcb to the UDP_PORT port */
   /* Using IP_ADDR_ANY allow the upcb to be used by any local interface */
   udp_bind(upcb, IP_ADDR_ANY, UDP_SERVER_PORT);
	
#if LCD_USER
	LCD_ShowString(60, 18*8, "New Port: 7", LGRAY, BLACK); 
#endif
   /* Set a receive callback for the upcb */
   udp_recv(upcb, udp_server_callback, NULL);
  
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
  struct tcp_pcb *pcb;
  __IO uint8_t iptab[4];
  uint8_t iptxt[20];
	int i;
	char Rec_date[90];
	char *ptr_a;
  
  /* We have received the UDP Echo from a client */
  /* read its IP address */
  iptab[0] = (uint8_t)((uint32_t)(addr->addr) >> 24);
  iptab[1] = (uint8_t)((uint32_t)(addr->addr) >> 16);
  iptab[2] = (uint8_t)((uint32_t)(addr->addr) >> 8);
  iptab[3] = (uint8_t)((uint32_t)(addr->addr));

  sprintf((char*)iptxt, "Ip is: %d.%d.%d.%d     ", iptab[3], iptab[2], iptab[1], iptab[0]);	                           	                             
	
  /* Display the client's IP address */
  /* If there is more than one client connected, the IP address of the last one connected will be displayed */
//  GLCD_displayStringLn(Line4, "Client's IP address");
//  GLCD_displayStringLn(Line5, iptxt);
 
  /* Connect to the remote client */
  udp_connect(upcb, addr, port);
	
#if LCD_USER
	ptr_a = ((char *)p->payload);
	
	for(i=0;((i<90) && (i<p->len));i++)
	  Rec_date[i] = *ptr_a++;

  Rec_date[i] = 0;

	
 	LCD_ShowString(0, 18*13, "Receive Date:", LGRAY, BLACK);
	LCD_ShowString(20, 18*14, iptxt, LGRAY, BLACK);
	LCD_ShowString(20, 18*15, "                                  ", LGRAY, BLACK);
	LCD_ShowString(20, 18*15, Rec_date, RED, BLACK);
	
#endif    
  /* Tell the client that we have accepted it */
  udp_send(upcb, p);
	


  

  /* Free the p buffer */
  pbuf_free(p);
   
}

/**
  * @brief  This funtion is called when a TCP connection has been established on the port TCP_PORT.
  * @param  arg	user supplied argument 
  * @param  pcb	the tcp_pcb which accepted the connection
  * @param  err error value
  * @retval ERR_OK
  */

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
