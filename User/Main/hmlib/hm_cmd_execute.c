/************************************************************************
 $Revision: 54 $                                                           
************************************************************************/
#include "hm_cmd_execute.h"
#include "hm_cmds_def.h"
#include "hm_cmd_type_def.h"
#include "hm_cmd_inter_uart.h"
#include "hm_cmd_inter_netwk.h"
#include "hm_step_motor.h"
#include "hm_step_motor_limit_operation.h"
#include "hm_common_cfg.h"
#include "hw_config.h"
#include "hm_sw_version.h"
#include "hm_uart.h"
#include "hm_temperature.h"
#include "hm_gpio.h"
#include "hm_led.h"
#include "hm_debug.h"
#include "hm_timer.h"
#include <stdio.h>
#include "hm_msg_queue.h"



/*static functions*/
static void hm_cmd_exc_ck(void*);

static void hm_cmd_reboot_ck(void* param)
{
	  hm_delay(500);
    NVIC_SystemReset();
}

void hm_cmd_exec_init(void)
{
#ifdef HM_CMD_METHOD_UART    
    hm_cmd_init();

    hm_cmd_register(hmCmdFixMove, hm_cmd_exc_ck);
    hm_cmd_register(hmCmdKeepMove, hm_cmd_exc_ck);
    hm_cmd_register(hmCmdStopMove, hm_cmd_exc_ck);
    hm_cmd_register(hmCmdGetAxisStatus, hm_cmd_exc_ck);   
    hm_cmd_register(hmCmdCmdLed, hm_cmd_exc_ck);      
#endif

#ifdef HM_CMD_METHOD_NETWK    
    hm_cmd_net_init();

    hm_cmd_net_register(hmCmdFixMove, hm_cmd_exc_ck);
    hm_cmd_net_register(hmCmdKeepMove, hm_cmd_exc_ck);
    hm_cmd_net_register(hmCmdStopMove, hm_cmd_exc_ck);
    hm_cmd_net_register(hmCmdGetAxisStatus, hm_cmd_exc_ck);   
    hm_cmd_net_register(hmCmdCmdLed, hm_cmd_exc_ck); 
    hm_cmd_net_register(hmCmdIo, hm_cmd_exc_ck); 
    hm_cmd_net_register(hmCmdEnableAxis, hm_cmd_exc_ck);   
    hm_cmd_net_register(hmCmdGetAxisSetting, hm_cmd_exc_ck); 
    hm_cmd_net_register(hmCmdSetPos, hm_cmd_exc_ck);     
    hm_cmd_net_register(hmCmdGetSystemInfo, hm_cmd_exc_ck);       
    hm_cmd_net_register(hmGetVersion, hm_cmd_exc_ck); 
    hm_cmd_net_register(hmResetBoard, hm_cmd_exc_ck); 
    hm_cmd_net_register(hmResetForLoad, hm_cmd_exc_ck);		
    hm_cmd_net_register(hmDebugOpe, hm_cmd_exc_ck);      
#endif    
}


void hm_cmd_exc_ck(void* param)
{
    hm_cmd_req_t *preq = (hm_cmd_req_t*)param;
    preq->ret = false;
    
    if(preq->ncmd == hmCmdFixMove)
    {
        hmCmdFixMove_t *pinf = (hmCmdFixMove_t*)preq->input;
        if(preq->input_len != sizeof(hmCmdFixMove_t))
        {
            preq->ret = false;
            return;
        }
        preq->ret = hm_step_motor_move(pinf->axis, pinf->step, pinf->accel, pinf->decel, pinf->speed);
    }
    else if(preq->ncmd == hmCmdKeepMove)
    {
        hmCmdKeepMove_t *pinf = (hmCmdKeepMove_t*)preq->input;
        if(preq->input_len != sizeof(hmCmdKeepMove_t))
        {
            preq->ret = false;
            return;
        }        
        if(pinf->bhome)
            preq->ret = hm_step_motor_start_homing(pinf->axis,  pinf->accel, pinf->speed);
        else
            preq->ret = hm_step_motor_start_free_run(pinf->axis,  pinf->accel, pinf->speed);
    }
    
    else if(preq->ncmd == hmCmdSetPos)
    {    
        hmCmdSetPos_t *pinf = (hmCmdSetPos_t*)preq->input;   
        hm_step_motor_set_cur_pos(pinf->axis, pinf->pos);
        preq->ret = true;
    }    
    else if(preq->ncmd == hmCmdEnableAxis)
    {    
        hmCmdEnableAxis_t *pinf = (hmCmdEnableAxis_t*)preq->input;   
        preq->ret = hm_motor_enable_axis(pinf->axis, pinf->benable);
    }
    else if(preq->ncmd == hmCmdGetSystemInfo)
    {    
        hmCmdGetSystemInfo_t *pinf = (hmCmdGetSystemInfo_t*)preq->input;   
        if(pinf->bset)
        {
            hm_uart_debug_off(pinf->bdebug_off);
        }
        else
        {
            pinf->bdebug_off    = hm_uart_is_debug_off();
            pinf->ntemperature  = DS18B20_Get_Temp();
            
        }
        preq->ret = true;
    }    
    else if(preq->ncmd == hmCmdStopMove)
    {
        hmCmdStopMove_t *pinf = (hmCmdStopMove_t*)preq->input;
        if(preq->input_len != sizeof(hmCmdStopMove_t))
        {
            preq->ret = false;
            return;
        }         
        if(pinf->bsmooth)
            preq->ret = hm_step_motor_stop_smooth(pinf->axis);
        else
            preq->ret = hm_step_motor_stop_sundden(pinf->axis);
    }
    else if(preq->ncmd == hmCmdGetAxisSetting)
    {
        hmCmdGetAxisSetting_t *pin   = (hmCmdGetAxisSetting_t*)preq->input;
        if(preq->input_len != sizeof(hmCmdGetAxisSetting_t))
        {
            preq->ret = false;
            return;
        }    
        
        if(pin->bget)
        {            
            if(pin->flag & hmsHomeTriger)   pin->bHomeTriger = hm_step_motor_get_home_stop_mode(pin->axis);  
            hm_motor_get_stop_mode(pin->axis, &pin->bStopModeCW, &pin->bStopModeCCW, &pin->bStopModeHome, &pin->Tcurve);            
        }
        else
        {
            uint32_t bstopModeCW, bstopModeCCW, bstopModeHome;
            uint8_t  bTCurve;
            hm_motor_get_stop_mode(pin->axis, &bstopModeCW, &bstopModeCCW, &bstopModeHome, &bTCurve);
            if(pin->flag & hmsHomeTriger)     hm_step_motor_set_home_stop_mode(pin->axis, pin->bHomeTriger); 
            if(pin->flag & hmsStopModeCW)     hm_motor_set_stop_mode(pin->axis, pin->bStopModeCW, bstopModeCCW, bstopModeHome, bTCurve); 
            if(pin->flag & hmsStopModeCCW)    hm_motor_set_stop_mode(pin->axis, bstopModeCCW, pin->bStopModeCCW, bstopModeHome, bTCurve); 
            if(pin->flag & hmsStopModeHome)   hm_motor_set_stop_mode(pin->axis, bstopModeCCW, bstopModeCCW, pin->bStopModeHome, bTCurve); 
            if(pin->flag & hmsCurve)          hm_motor_set_stop_mode(pin->axis, bstopModeCCW, bstopModeCCW, bstopModeHome, pin->Tcurve); 
        }
        preq->ret = true;
    }
    else if(preq->ncmd == hmCmdGetAxisStatus)
    {
        hmCmdGetAxisStatuse_t *pin   = (hmCmdGetAxisStatuse_t*)preq->input;
        if(preq->input_len != sizeof(hmCmdGetAxisStatuse_t))
        {
            preq->ret = false;
            return;
        }           
        pin->axis_status                = 0;
        pin->pos                        = hm_step_motor_get_cur_pos(pin->axis);
        pin->speed                      = hm_step_motor_get_curr_speed(pin->axis);
        if(hm_step_motor_state(pin->axis) == rsSTOP)    pin->axis_status |= hmsStoped;
        if(hm_motor_hit_CW(pin->axis))                  pin->axis_status |= hmsHitCW;
        if(hm_motor_hit_CCW(pin->axis))                 pin->axis_status |= hmsHitCCW;
        if(hm_motor_hit_Home(pin->axis))                pin->axis_status |= hmsHitHome;
        if(hm_motor_is_enable(pin->axis))               pin->axis_status |= hmsEnabled;
        preq->ret                       = true;
        
    }  
    else if(preq->ncmd == hmCmdCmdLed)
    {
        hmCmdCmdLed_t *pin   = (hmCmdCmdLed_t*)preq->input;
        if(preq->input_len != sizeof(hmCmdCmdLed_t))
        {
            preq->ret = false;
            return;
        }           
        
        HM_NET_PRINT("hmCmdCmdLed idx= %d, bon=%d\t\n", pin->idx, pin->bon);
        if(pin->idx == 1) hm_led_act(ltCh1, pin->bon);
        if(pin->idx == 2) hm_led_act(ltCh2, pin->bon);
        if(pin->idx == 3) hm_led_act(ltCh3, pin->bon);
        if(pin->idx == 4) hm_led_act(ltCh4, pin->bon);
        preq->ret = true;
    }  
    else if(preq->ncmd == hmCmdIo)
    {  
        hmCmdIo_t *pin   = (hmCmdIo_t*)preq->input;
        if(preq->input_len != sizeof(hmCmdIo_t))
        {
            preq->ret = false;
            return;
        }           
        
        pin->val_ret = 0;
        if(pin->bout)
        {
            if(pin->bread)
                pin->val_ret = hm_gpio_get_out_bit(pin->idx);
            else 
                hm_gpio_out_bit(pin->idx, pin->val_set);
        }
        else
        {
            pin->val_ret = hm_gpio_in_bit(pin->idx);
        }
//        HM_NET_PRINT("hmCmdIo idx= %d, bout =%d, bread=%d, val_set=%d, val_ret =%d\n", pin->idx, pin->bout, pin->bread, pin->val_set, pin->val_ret);
        
        preq->ret = true;
        if(pin->val_ret == 0xffff) preq->ret = false;

    }
    else if(preq->ncmd == hmGetVersion)
    {  
        hmGetVersion_t *pin   = (hmGetVersion_t*)preq->input;
        if(preq->input_len != sizeof(hmGetVersion_t))
        {
            preq->ret = false;
            return;
        }       
        pin->nversion      = SW_VERSION;    
        preq->ret = true;
    }
    else if(preq->ncmd == hmDebugOpe)
    {  
        hmDebugOpe_t *pin   = (hmDebugOpe_t*)preq->input;
        if(preq->input_len != sizeof(hmDebugOpe_t))
        {
            preq->ret = false;
            return;
        }       
        hm_cmd_net_init_debug(pin->bdebug_on, pin->nport);
        preq->ret = true;
    }   
    else if(preq->ncmd == hmResetForLoad)
    {		
			  hm_msg_queue_add(hm_cmd_reboot_ck, 0);
				
		}
    else if(preq->ncmd == hmResetBoard)
    {  
        uint32_t idx;
        for(idx = 0; idx < HM_MAX_GPIO_OUT; ++idx)
        {
            hm_gpio_out_bit(idx, 0);
        }
        for(idx = 0; idx < HM_MAX_AXIS; ++idx)
        {
            hm_step_motor_stop_smooth(idx);
            hm_step_motor_set_cur_pos(idx, 0);
        }       
        preq->ret = true;
    }   
}
