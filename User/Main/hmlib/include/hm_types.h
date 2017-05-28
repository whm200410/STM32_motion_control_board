#ifndef __HM_TYPES_H
#define __HM_TYPES_H


typedef void(*hm_ck_func_t)(void*);

#ifndef NULL
#define NULL    0
#endif

typedef unsigned int bool;

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif


typedef enum 
{
    rtSuccuss       = 0x00,
    rtGeneError     = 0x01,
    rtAlreadyRun    = 0x02,
}RET_TYPE;

#endif
