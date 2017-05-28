#ifndef __HM_temperature_H
#define __HM_temperature_H
#include "hm_types.h"
uint8_t DS18B20_Init(void);
int16_t DS18B20_Get_Temp(void);
#endif
