/************************************************************************
 $Revision$
************************************************************************/

#ifndef __HM_DEBUG_H
#define __HM_DEBUG_H
#include "hm_types.h"
#include <stdint.h>
#include <stdio.h>
#include "hm_timer.h"
#include "hm_debug_private.h"
#include "hm_common_cfg.h"


/*similar to printf but can be called in irp hander(push actual print to main loop)*/

#define HM_NET_PRINT(sz_format, ...)							                    \
do																                    \
{																                    \
    if(hm_net_isdebugon())                                                          \
    {                                                                               \
    hm_print_buf_t* ppnt_buf = hm_net_find_idle_buf();                              \
    if(ppnt_buf)                                                                    \
    {                                                                               \
        int nlen = 0;                                                               \
        int nlen2 = 0;                                                              \
        nlen2 = sprintf(&ppnt_buf->sbuf[0], "cl:%-10d  ",hm_get_cur_time());     \
        nlen  = snprintf(&ppnt_buf->sbuf[nlen2], HM_UART_PRINT_BUF_SIZE, sz_format, __VA_ARGS__);   \
        nlen += nlen2;                                                              \
        ppnt_buf->sbuf[nlen] = '\n';                                                \
        ppnt_buf->sbuf[nlen+1] = '\0';                                              \
        ppnt_buf->len = nlen+1;                                                     \
        hm_net_add_print_pending(ppnt_buf);                                         \
    }                                                                               \
    }                                                                               \
} while (0)


#endif
