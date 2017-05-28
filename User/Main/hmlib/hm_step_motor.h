/************************************************************************
 $Revision: 54 $                                                           
************************************************************************/
#ifndef __HM_STEP_MOTOR_H
#define __HM_STEP_MOTOR_H
#include "hm_types.h"
#include "hm_step_motor_defs.h"


/*init stepper motor control*/
void hm_step_motor_init(void);

/**
  * @brief  start to run at specific speed to home postion
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @param  accel: p/s2
  * @param  speed: p/s 
  * @retval None
  */
bool        hm_step_motor_start_homing(uint32_t axis,  uint32_t accel, int32_t speed);


/**
  * @brief  start to run at specific speed
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @param  accel: p/s2
  * @param  speed: p/s 
  * @retval None
  */
bool        hm_step_motor_start_run(uint32_t axis,  uint32_t accel, int32_t speed);

bool        hm_step_motor_start_free_run(uint32_t axis,  uint32_t accel, int32_t speed);

/**
  * @brief  move to specific step, if axis is running, need to stop first, not allowed to continue move
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @param  step: how much pulse
  * @param  accel: p/s2
  * @param  decel: p/s2  
  * @param  speed: p/s 
  * @retval None
  */
bool        hm_step_motor_move(uint32_t axis, int32_t step, uint32_t accel,uint32_t decel, uint32_t speed);

/**
  * @brief  stoped or runing
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval RUNING or STOPPED
  */
RUN_STATE_E hm_step_motor_state(uint32_t axis);
AXIS_STATE_E hm_step_axis_state(uint32_t axis);

/**
  * @brief  none meaning
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval none meaning
  */
uint32_t    hm_step_motor_last_actual_steps_moved(uint32_t axis);


/**
  * @brief  stop motor smoothly, from current speed to 0
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval None
  */
bool        hm_step_motor_stop_smooth(uint32_t axis);

/**
  * @brief  stop motor sunddenly
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval None
  */
bool        hm_step_motor_stop_sundden(uint32_t axis);

/**
  * @brief  get current speed and position
  * @param  axis: from 0 to HM_MAX_AXIS - 1.
  * @retval None
  */
uint32_t    hm_step_motor_get_curr_speed(uint32_t axis);
int32_t     hm_step_motor_get_cur_pos(uint32_t axis);
void        hm_step_motor_set_cur_pos(uint32_t axis, int32_t newpos);
void        hm_step_motor_set_home_stop_mode(uint32_t axis, bool blowtriger);
bool        hm_step_motor_get_home_stop_mode(uint32_t axis);
void        hm_step_motor_set_axis_curve(uint32_t axis, methrd_e e);
methrd_e    hm_step_motor_get_axis_curve(uint32_t axis);
#endif
