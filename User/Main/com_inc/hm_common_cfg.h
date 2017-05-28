#ifndef __HM_COMMON_CFG_H
#define __HM_COMMON_CFG_H



#define     HM_UART_PRINT_BUF_SIZE       200

/*struct used to do delay print*/
typedef struct hm_print_buf_s
{
    uint32_t    len;
    char sbuf[HM_UART_PRINT_BUF_SIZE];    
}hm_print_buf_t;



#define HM_IP_START                         {192, 168, 1, 200}
/*udp server port*/
#define HM_UDP_PORT                         5987
#define HM_UDP_PORT_LOG                     5988
#define HM_UDP_PORT_LOG_HOST                4695

/*max axis number*/
#define HM_MAX_AXIS                         4

/*max gpio in number*/
#define HM_MAX_GPIO_OUT                     12

/*max gpio out number*/
#define HM_MAX_GPIO_IN                      8

/*used to define the communicate interface between host and firmware,
HM_CMD_METHOD_UART is for usart, HM_CMD_METHOD_NETWK is used for network*/
#define xHM_CMD_METHOD_UART

#ifndef HM_CMD_METHOD_UART
#define HM_CMD_METHOD_NETWK
#endif

#endif
