/************************************************************************
 $Revision$                                                           
************************************************************************/

#ifndef __HM_IWDG_H
#define __HM_IWDG_H
#include "hm_types.h"

/**
  * @brief  Init watch dog ,should feed per 500ms
  * @param  none
  * @retval if reset from wdg, return true, else return false
  */
bool hm_iwdg_init(void);


/**
  * @brief  should feed per 500ms
  * @param  none
  * @retval none
  */
void hm_iwdg_feed(void*);
#endif
