/************************************************************************
 $Revision: 55 $                                                           
************************************************************************/
#include "stm32f10x.h"
#include "hm_step_motor.h"
#include "hm_step_motor_defs.h"
#include "hw_config.h"
#include "hm_step_motor_limit_operation.h"
#include "hm_common_cfg.h"
#include "hm_uart.h"
#include "hm_led.h"
#include "hm_debug.h"
hm_step_info_t   s_axis_info[HM_MAX_AXIS];

//init date structure
static void hm_motor_data_init(uint32_t idx );

//init hw resource such as gpio, pins, dma channels, tim defs
static void hm_motor_hw_info_init(uint32_t idx );

//init timer and dma based on hw resources for all axis
static void hm_motor_tim_init(void);

//call back from dma done interrupt
static void hm_dma_ck(uint32_t axis);

//update the delays
static void hm_dma_update(hm_step_info_t *psrd, hm_dma_buf_info_t *pdma_buf);

//setup dma channel information used to set tim compare pwm
static void hm_dma_setup(hm_step_info_t *psrd, bool benable);

//start or stop timer
static void hm_tim_start(hm_step_info_t *psrd, bool benable, uint64_t fre);

static void hm_tim_pause_resume(hm_step_info_t *psrd, bool bpause);

static void hm_mark_axis_state(int axis, AXIS_STATE_E e);
static uint32_t mysqrt(uint32_t x);
static uint64_t tim_base_fre(uint32_t speed);
void hm_step_motor_init(void)
{
    uint32_t                idx = 0;
    for(idx = 0; idx < HM_MAX_AXIS; ++idx)
    {
        hm_motor_hw_info_init(idx);
        hm_motor_data_init(idx);
    }
    idx = sizeof(hm_step_info_t);
    hm_motor_tim_init();
}

void hm_motor_hw_info_init(uint32_t idx)
{
    hm_motor_hw_limit_config(idx);
    s_axis_info[idx].rcc_mod            = 0;
    s_axis_info[idx].rcc_mod1           = 0;
    s_axis_info[idx].rcc_mod2           = 0;
    s_axis_info[idx].nhw_ch             = 1; 
    s_axis_info[idx].remap              = 0;    
    s_axis_info[idx].idx                = idx;
    s_axis_info[idx].method             = TCurve;
    hm_step_motor_set_home_stop_mode(idx, true);
    if(idx == 0)
    {
        s_axis_info[idx].irq            = DMA1_Channel5_IRQn;
        s_axis_info[idx].port_pul       = GPIOE;
        s_axis_info[idx].ptim           = TIM1;
        s_axis_info[idx].pul_pin        = GPIO_Pin_9;
        s_axis_info[idx].pdma           = DMA1_Channel5;
        s_axis_info[idx].rcc_mod2       = RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO;
        s_axis_info[idx].rcc_mod        = RCC_AHBPeriph_DMA1;
        s_axis_info[idx].remap          = GPIO_FullRemap_TIM1;
        s_axis_info[idx].nhw_ch         = 1; 
    }   
    else if(idx == 1)   
    {   
        s_axis_info[idx].irq            = DMA1_Channel2_IRQn;
        s_axis_info[idx].port_pul       = GPIOA;
        s_axis_info[idx].ptim           = TIM2;
        s_axis_info[idx].pdma           = DMA1_Channel2;
        s_axis_info[idx].pul_pin        = GPIO_Pin_0;
        s_axis_info[idx].rcc_mod2       = RCC_APB2Periph_GPIOA;
        s_axis_info[idx].rcc_mod1       = RCC_APB1Periph_TIM2;
        s_axis_info[idx].rcc_mod        = RCC_AHBPeriph_DMA1;
        s_axis_info[idx].nhw_ch         = 1; 
    }   
    else if(idx == 2)   
    {   
        s_axis_info[idx].irq            = DMA1_Channel3_IRQn;
        s_axis_info[idx].port_pul       = GPIOA;
        s_axis_info[idx].ptim           = TIM3;
        s_axis_info[idx].pdma           = DMA1_Channel3;
        s_axis_info[idx].pul_pin        = GPIO_Pin_6;
        s_axis_info[idx].rcc_mod1       = RCC_APB1Periph_TIM3; 
        s_axis_info[idx].rcc_mod        = RCC_AHBPeriph_DMA1;
    }   
    else if(idx == 3)
    {
        s_axis_info[idx].irq            = DMA1_Channel7_IRQn;
        s_axis_info[idx].port_pul       = GPIOB;
        s_axis_info[idx].ptim           = TIM4;
        s_axis_info[idx].pdma           = DMA1_Channel7;
        s_axis_info[idx].pul_pin        = GPIO_Pin_6;
        s_axis_info[idx].rcc_mod1       = RCC_APB1Periph_TIM4; 
        s_axis_info[idx].rcc_mod        = RCC_AHBPeriph_DMA1;
        s_axis_info[idx].rcc_mod2       = RCC_APB2Periph_GPIOB;
    }       
    s_axis_info[idx].rcc_mod1           |= s_axis_info[idx].motor_hw_config.rcc_mod1;
    s_axis_info[idx].rcc_mod2           |= s_axis_info[idx].motor_hw_config.rcc_mod2;
    
     
}
void hm_mark_axis_state(int axis, AXIS_STATE_E e)
{
    s_axis_info[axis].axis_state = e;
    HM_NET_PRINT("hm_mark_axis_state(axis = %d,  AXIS_STATE_E = %d", axis, (int)e);
}
void hm_motor_data_init(uint32_t idx)
{
    uint32_t idx_dma_buf                = 0;

    hm_mark_axis_state(idx, asStoped); 
    s_axis_info[idx].run_state          = rsSTOP;
    s_axis_info[idx].dir                = rdCW;
    s_axis_info[idx].pos                = 0;
    s_axis_info[idx].HM_STEP_FRE        = HM_STEP_FRE_INIT;   
    s_axis_info[idx].stop_accel         = HM_STOP_ACCEL_VALUE;
    s_axis_info[idx].dma_fill           = 0;
    for(idx_dma_buf = 0; idx_dma_buf < HM_DMA_BUF_BLOCK_SZ; ++idx_dma_buf)
    {
        s_axis_info[idx].dma_buf[idx_dma_buf].valid = 0;
        s_axis_info[idx].dma_buf[idx_dma_buf].dma_buf_len = 0;
    }
    s_axis_info[idx].dma_buf_idx = 0;
}
void hm_motor_tim_init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;
    GPIO_InitTypeDef        GPIO_InitStructure;
    DMA_InitTypeDef         DMA_InitStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;
    uint32_t                idx = 0;
    for(idx = 0; idx < HM_MAX_AXIS; ++idx)
    {
        /*clock set*/
        if(s_axis_info[idx].rcc_mod)RCC_AHBPeriphClockCmd(s_axis_info[idx].rcc_mod, ENABLE);        
        if(s_axis_info[idx].rcc_mod2)RCC_APB2PeriphClockCmd(s_axis_info[idx].rcc_mod2, ENABLE);
        if(s_axis_info[idx].rcc_mod2)RCC_APB1PeriphClockCmd(s_axis_info[idx].rcc_mod1, ENABLE);
        if(s_axis_info[idx].remap) 
            GPIO_PinRemapConfig(s_axis_info[idx].remap, ENABLE);
        
        /*tim1 cc1 and tim2 gpio set*/
        GPIO_StructInit (&GPIO_InitStructure);
        
        GPIO_InitStructure.GPIO_Pin         =  s_axis_info[idx].pul_pin;
        GPIO_InitStructure.GPIO_Mode        = GPIO_Mode_AF_PP;
        GPIO_InitStructure.GPIO_Speed       = GPIO_Speed_50MHz;
        GPIO_Init(s_axis_info[idx].port_pul, &GPIO_InitStructure);  

        /* TIM base*/
        TIM_DeInit(s_axis_info[idx].ptim);
        TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
        
        TIM_TimeBaseStructure.TIM_Period        = 0xFFFF;          
        TIM_TimeBaseStructure.TIM_Prescaler     = (uint16_t) ((uint64_t)SystemCoreClock / s_axis_info[idx].HM_STEP_FRE) - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;
        TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;   
        TIM_TimeBaseInit(s_axis_info[idx].ptim, &TIM_TimeBaseStructure);


        TIM_OCStructInit (&TIM_OCInitStructure);
        
        /* TIM Configuration in PWM Mode */

        TIM_OCInitStructure.TIM_OCMode          =  TIM_OCMode_PWM1;   
        TIM_OCInitStructure.TIM_OutputState     = TIM_OutputState_Enable;          
        TIM_OCInitStructure.TIM_Pulse           = 0xFFF;  
        
        if(s_axis_info[idx].nhw_ch == 1)
            TIM_OC1Init(s_axis_info[idx].ptim, &TIM_OCInitStructure); 
        else if(s_axis_info[idx].nhw_ch == 2)
            TIM_OC2Init(s_axis_info[idx].ptim, &TIM_OCInitStructure); 
        else if(s_axis_info[idx].nhw_ch == 3)
            TIM_OC3Init(s_axis_info[idx].ptim, &TIM_OCInitStructure); 
        else if(s_axis_info[idx].nhw_ch == 4)
            TIM_OC4Init(s_axis_info[idx].ptim, &TIM_OCInitStructure);         
        
        TIM_ARRPreloadConfig(s_axis_info[idx].ptim, DISABLE);
        /* TIM1 DMAR Base register and DMA Burst Length Config */
        TIM_DMAConfig(s_axis_info[idx].ptim, TIM_DMABase_ARR, TIM_DMABurstLength_3Transfers);
        
        /* TIM1 DMA Update enable */
        TIM_DMACmd(s_axis_info[idx].ptim, TIM_DMA_Update, ENABLE);
        TIM_CtrlPWMOutputs(s_axis_info[idx].ptim, ENABLE);
//        TIM_ITConfig(s_axis_info[idx].ptim, TIM_IT_Update, ENABLE);
        
        /*init DMA*/
        DMA_DeInit(s_axis_info[idx].pdma); /* DMA1 Channel5 is for TIM1 */
        DMA_InitStructure.DMA_PeripheralBaseAddr        = (uint32_t)&s_axis_info[idx].ptim->DMAR; 
        DMA_InitStructure.DMA_MemoryBaseAddr            = (uint32_t)s_axis_info[idx].dma_buf[0].dma_buf; 
        DMA_InitStructure.DMA_DIR                       = DMA_DIR_PeripheralDST;
        DMA_InitStructure.DMA_BufferSize                = HM_DMA_BUF_UNIT_SIZE * HM_DMA_BUF_SIZE;
        DMA_InitStructure.DMA_PeripheralInc             = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc                 = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize        = DMA_PeripheralDataSize_HalfWord;
        DMA_InitStructure.DMA_MemoryDataSize            = DMA_MemoryDataSize_HalfWord;
        DMA_InitStructure.DMA_Mode                      = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority                  = DMA_Priority_High;
        DMA_InitStructure.DMA_M2M                       = DMA_M2M_Disable;
        DMA_Init(s_axis_info[idx].pdma, &DMA_InitStructure);  

        DMA_ITConfig(s_axis_info[idx].pdma, DMA_IT_TC | DMA_IT_HT, ENABLE);
   //     DMA_Cmd(s_axis_info[idx].pdma,  ENABLE );
        
        
        /*NVIC*/
        NVIC_InitStructure.NVIC_IRQChannel                      = s_axis_info[idx].irq; 		  //USART1接收中断
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 0; //先占优先级0
        NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 0;		  //次占优先级
        NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
        hm_motor_hw_limit_init(idx);

    }
    /* TIM1 PWM Outputs Enable */
    
}
bool        hm_step_motor_start_homing(uint32_t axis,  uint32_t accel, int32_t speed)
{
    hm_step_info_t      *psrd   = &s_axis_info[axis];       
    psrd->bdo_home              = true;
    HM_NET_PRINT("hm_step_motor_start_homing(axis = %d,  accel = %d, speed = %d, pos = %d)\n", axis, accel, speed, psrd->pos);
    return hm_step_motor_start_run(axis, accel, speed);
}
bool hm_step_motor_start_free_run(uint32_t axis,  uint32_t accel, int32_t speed)
{
    hm_step_info_t      *psrd   = &s_axis_info[axis];       
    psrd->bdo_home              = false;
    HM_NET_PRINT("hm_step_motor_start_free_run(axis = %d,  accel = %d, speed = %d, pos = %d)\n", axis, accel, speed, psrd->pos);
    return hm_step_motor_start_run(axis, accel, speed); 
}
bool hm_step_motor_start_run(uint32_t axis,  uint32_t accel, int32_t speed)
{
    hm_step_info_t      *psrd = NULL;
    hm_dma_buf_info_t   *pdma_buf = NULL;
    //! Number of steps before we hit max speed.
    uint32_t            max_s_lim;
    //! Number of steps before we must start deceleration (if accel does not hit max speed).
    uint32_t            dma_buf_idx;
 //   uint64_t            HM_STEP_FRE_DIV_10000_MUL_0676;
    
    if(axis >= HM_MAX_AXIS || hm_step_motor_state(axis) != rsSTOP) 
    {
        HM_NET_PRINT("!!start_run return because still running axis = %d", axis);
        return false;
    }
    
    
    psrd                = &s_axis_info[axis];
    pdma_buf            = psrd->dma_buf;
    
        
    if(speed <0)
    {
        speed = -speed;
        psrd->dir = rdCCW;
        
    }
    else
    {
        psrd->dir = rdCW;
    }
    if(hm_motor_hit_CW(axis) && psrd->dir == rdCW) 
    {
        HM_NET_PRINT("!!start_run return because hm_motor_hit_CW and rdCW axis = %d", axis);
        return true;
    }
    if(hm_motor_hit_CCW(axis) && psrd->dir == rdCCW) 
    {
        HM_NET_PRINT("!!start_run return because hm_motor_hit_CCW and rdCCW axis = %d", axis);
        return true;
    }
    if(hm_motor_hit_Home(axis) && psrd->bdo_home && psrd->bhome_stop_low_triger) 
    {
        HM_NET_PRINT("!!start_run return because hm_motor_hit_Home and bdo_home and low triger axis = %d", axis);
        return true;
    }
    hm_mark_axis_state(axis, asPreparing); 
    if(psrd->bdo_home) psrd->motor_hw_config.blast_hit_home = hm_motor_hit_Home(axis);
    
    
    hm_motor_enable_axis(axis, true);
    psrd->HM_STEP_FRE       = tim_base_fre(speed);
    psrd->steps_request     = HM_INVALID_STEP;
    psrd->steps_taken       = HM_INVALID_STEP;
    psrd->steps_actual      = HM_INVALID_STEP;
    psrd->steps_tim_irq     = 0;   
    psrd->bstopping         = false;  
    psrd->rest              = 0;
    for(dma_buf_idx = 0; dma_buf_idx < HM_DMA_BUF_BLOCK_SZ; ++dma_buf_idx)
    {
        psrd->dma_buf[dma_buf_idx].valid = 0;
        psrd->dma_buf[dma_buf_idx].dma_buf_len = 0;
    }
    psrd->dma_buf_idx       = 0;
    psrd->min_step_delay    = 0xffff;
    psrd->dma_irq_idx       = 0;
    // Set direction from sign on step value.

    hm_motor_set_dir(axis, psrd->dir == rdCW);
    

    // Refer to documentation for detailed information about these calculations.

    // Set max speed limit, by calc min_delay to use in timer.
    // min_delay = (alpha / tt)/ w
    psrd->min_delay     = (uint16_t)(psrd->HM_STEP_FRE / speed);

    // Set accelration by calc the first (c0) step delay .
    // step_delay = 1/tt * sqrt(2*alpha/accel)
    // step_delay = ( tfreq*0.676/100 )*100 * sqrt( (2*alpha*10000000000) / (accel*100) )/10000
    psrd->step_delay    = (uint16_t)((psrd->HM_STEP_FRE / 10000) *0.676 * mysqrt(200000000/accel));
    // Find out after how many steps does the speed hit the max speed limit.
    // max_s_lim = speed^2 / (2*alpha*accel)
    if(psrd->method == sCurve)
        max_s_lim           = (uint32_t)(((uint64_t)(speed)) * 1.358695652173913 * speed / accel);
    else
        max_s_lim           = (uint32_t)(((uint64_t)(speed)) / 2 * speed / accel);    
    psrd->n2            = max_s_lim;
    psrd->accel_steps   = max_s_lim;
    psrd->decel_start   = HM_INVALID_STEP;

    // If we hit max speed limit before 0,5 step it will round to 0.
    // But in practice we need to move atleast 1 step to get any speed at all.
    if(max_s_lim == 0){
        max_s_lim = 1;
    }

    // If the maximum speed is so low that we dont need to go via accelration state.
    if(psrd->step_delay <= psrd->min_delay)
    {
        psrd->step_delay = psrd->min_delay;
        psrd->run_state = rsRUN;
    }
    else{
        psrd->run_state = rsACCEL;
    }

    // Reset counter.
    psrd->accel_count = 0;
    /*update the delay for all dma buffers*/
    pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = psrd->step_delay - 1;
    pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = 1;
    pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = psrd->step_delay / 2;
    psrd->steps_taken = 1;
    hm_dma_update(psrd, pdma_buf);
    for(dma_buf_idx = psrd->dma_buf_idx + 1; dma_buf_idx < HM_DMA_BUF_BLOCK_SZ && psrd->run_state != rsSTOP; ++dma_buf_idx)
    {
        hm_dma_update(psrd, &pdma_buf[dma_buf_idx]);
    }
    
    hm_dma_setup(psrd, true);
    hm_tim_start(psrd, true, psrd->HM_STEP_FRE);
    
    HM_NET_PRINT("!end hm_step_motor_start_run axis = %d, pos = %d ethod = %d", axis, psrd->pos, psrd->method);
    return true;


}

bool hm_step_motor_move(uint32_t axis, int32_t step, uint32_t accel,uint32_t decel, uint32_t speed)
{
    hm_step_info_t      *psrd = NULL;
    hm_dma_buf_info_t   *pdma_buf = NULL;
    //! Number of steps before we hit max speed.
    uint32_t            max_s_lim;
    //! Number of steps before we must start deceleration (if accel does not hit max speed).
    uint32_t            accel_lim;
    uint32_t            dma_buf_idx;
    
    if(axis >= HM_MAX_AXIS || hm_step_motor_state(axis) != rsSTOP) 
    {
        HM_NET_PRINT("!!start_run return because still running axis = %d", axis);
        return false;
    }
    
    psrd                = &s_axis_info[axis];
    pdma_buf            = psrd->dma_buf;
    
        
    if(step <0)
    {
        step = -step;
        psrd->dir = rdCCW;
    }
    else
    {
        psrd->dir = rdCW;

    }
    if(hm_motor_hit_CW(axis) && psrd->dir == rdCW) 
    {
        HM_NET_PRINT("!!start_run return because hm_motor_hit_CW and rdCW axis = %d", axis);
        return true;
    }
    if(hm_motor_hit_CCW(axis) && psrd->dir == rdCCW) 
    {
        HM_NET_PRINT("!!start_run return because hm_motor_hit_CCW and rdCCW axis = %d", axis);
        return true;
    }
    if(hm_motor_hit_Home(axis) && psrd->bdo_home && psrd->bhome_stop_low_triger) 
    {
        HM_NET_PRINT("!!start_run return because hm_motor_hit_Home and bdo_home and low triger axis = %d", axis);
        return true;
    }
    hm_motor_enable_axis(axis, true);
    psrd->HM_STEP_FRE       = tim_base_fre(speed);   
    psrd->steps_request     = step;
    psrd->steps_taken       = 0;
    psrd->steps_actual      = 0;
    psrd->steps_tim_irq     = 0;   
    psrd->bstopping         = false;  
    psrd->bdo_home          = false;  
    hm_mark_axis_state(axis, asPreparing);
    for(dma_buf_idx = 0; dma_buf_idx < HM_DMA_BUF_BLOCK_SZ; ++dma_buf_idx)
    {
        psrd->dma_buf[dma_buf_idx].valid = 0;
        psrd->dma_buf[dma_buf_idx].dma_buf_len = 0;
    }
    psrd->dma_buf_idx       = 0;
    psrd->min_step_delay    = 0xffff;
    psrd->dma_irq_idx       = 0;
    psrd->rest              = 0;
    // Set direction from sign on step value.

    hm_motor_set_dir(axis, psrd->dir == rdCW);
    // If moving only 1 step.
    if(step == 1)
    {
        // Move one step...
        psrd->accel_count = -1;
        // ...in DECEL state.
        psrd->run_state = rsDECEL;
        // Just a short delay so main() can act on 'running'.
        psrd->step_delay = 1000;
        
        pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = psrd->step_delay - 1;
        pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = 1;
        pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = psrd->step_delay / 2;
        pdma_buf->valid = 1;
        psrd->steps_taken = 1;
    }
    // Only move if number of steps to move is not zero.
    else if(step != 0)
    {
        // Refer to documentation for detailed information about these calculations.

        // Set max speed limit, by calc min_delay to use in timer.
        // min_delay = (alpha / tt)/ w
        psrd->min_delay     = (uint16_t)(psrd->HM_STEP_FRE / speed);

        // Set accelration by calc the first (c0) step delay .
        // step_delay = 1/tt * sqrt(2*alpha/accel)
        // step_delay = ( tfreq*0.676/100 )*100 * sqrt( (2*alpha*10000000000) / (accel*100) )/10000
        psrd->step_delay    = (psrd->HM_STEP_FRE / 10000) *0.676 * mysqrt(200000000/accel);
        // Find out after how many steps does the speed hit the max speed limit.
        // max_s_lim = speed^2 / (2*alpha*accel)
        if(psrd->method == sCurve)
            max_s_lim           = (uint32_t)(((uint64_t)(speed)) * 1.358695652173913 * speed / accel);
        else
            max_s_lim           = (uint32_t)(((uint64_t)(speed)) / 2 * speed / accel);    
        psrd->n2                = max_s_lim;
        // If we hit max speed limit before 0,5 step it will round to 0.
        // But in practice we need to move atleast 1 step to get any speed at all.
        if(max_s_lim == 0){
            max_s_lim = 1;
        }

        // Find out after how many steps we must start deceleration.
        // n1 = (n1+n2)decel / (accel + decel)
        accel_lim           = step / 2;//(step*(decel / 100)) / ((accel+decel) / 100);
        // We must accelrate at least 1 step before we can start deceleration.
        if(accel_lim == 0){
            accel_lim = 1;
        }

        // Use the limit we hit first to calc decel.
        if(accel_lim <= max_s_lim){
            psrd->decel_val = accel_lim - step;
        }
        else{
            psrd->decel_val =-(max_s_lim);
        }
        // We must decelrate at least 1 step to stop.
        if(psrd->decel_val == 0){
            psrd->decel_val = -1;
        }

        // Find step to start decleration.
        psrd->decel_start = step + psrd->decel_val;

        // If the maximum speed is so low that we dont need to go via accelration state.
        if(psrd->step_delay <= psrd->min_delay)
        {
            psrd->step_delay = psrd->min_delay;
            psrd->run_state = rsRUN;
        }
        else{
            psrd->run_state = rsACCEL;
        }

        // Reset counter.
        psrd->accel_count = 0;
        /*update the delay for all dma buffers*/
        pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = psrd->step_delay - 1;
        pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = 0;
        pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = psrd->step_delay / 2;
        psrd->steps_taken = 1;
        hm_dma_update(psrd, pdma_buf);
        for(dma_buf_idx = psrd->dma_buf_idx + 1; dma_buf_idx < HM_DMA_BUF_BLOCK_SZ && psrd->run_state != rsSTOP; ++dma_buf_idx)
        {
            hm_dma_update(psrd, &pdma_buf[dma_buf_idx]);
        }
 //       HM_NET_PRINT("min_delay=%d, step_delay=%d, max_s_lim = %d, decel_val=%d, decel_start=%d\r\n", psrd->min_delay, psrd->step_delay, max_s_lim,
 //           psrd->decel_val, psrd->decel_start);        
    }
    hm_dma_setup(psrd, true);
    hm_tim_start(psrd, true, psrd->HM_STEP_FRE);
    
    HM_NET_PRINT("hm_step_motor_move(axis = %d, step = %d, accel = %d,decel = %d, speed = %d, pos = %d) method = %d\r\n", 
        axis, step, accel, accel, speed, psrd->pos, psrd->method);
    return true;
}
void hm_dma_update(hm_step_info_t *psrd, hm_dma_buf_info_t *pdma_buf)
{
    uint32_t        step_count;
    uint16_t        new_step_delay;
    int32_t         div1;
    int32_t         div2;

    step_count = psrd->steps_taken;
    while(pdma_buf->dma_buf_len < HM_DMA_BUF_SIZE * HM_DMA_BUF_UNIT_SIZE)
    {

        if(psrd->steps_request != HM_INVALID_STEP)
        {
            if(step_count >= psrd->steps_request) break;
            else step_count++;
        }
        
        if(psrd->run_state == rsACCEL)
        {
            psrd->accel_count++;
            
            if(psrd->method == sCurve)
            {
                if(psrd->n2 == psrd->accel_count || (psrd->n2 + 1) == psrd->accel_count)
                {
                    psrd->rest          = 0;
                }
            }
            if(psrd->method == sCurve && psrd->n2 > psrd->accel_count)
            {
                div1 = 2 * (int32_t)psrd->step_delay * (psrd->n2 - psrd->accel_count) + psrd->rest;
                div2 = (4 * psrd->accel_count + 1) * psrd->n2;
            }
            else
            {
                div1 = 2 * (int32_t)psrd->step_delay + psrd->rest;
                div2 = 4 * psrd->accel_count + 1;
            }

            new_step_delay          = psrd->step_delay - div1 / div2;
            psrd->rest              = div1 % div2;
            if(psrd->decel_start != HM_INVALID_STEP && step_count >= psrd->decel_start)
            {
                psrd->accel_count   = psrd->decel_val;
                psrd->run_state     = rsDECEL;
            }
            // Chech if we hitted max speed.
            else if(new_step_delay <= psrd->min_delay)
            {
                psrd->last_accel_delay    = new_step_delay;
                new_step_delay      = psrd->min_delay;
                psrd->rest          = 0;
                psrd->run_state     = rsRUN;
            }
        }
        else if(psrd->run_state == rsRUN)
        {
            new_step_delay          = psrd->min_delay;
            // Chech if we should start decelration.
            if(psrd->decel_start != HM_INVALID_STEP && step_count >= psrd->decel_start)
            {
                psrd->accel_count   = psrd->decel_val;
                // Start decelration with same delay as accel ended with.
                new_step_delay      = psrd->last_accel_delay;
                psrd->run_state     = rsDECEL;
            }
        }
        else if(psrd->run_state == rsDECEL)
        {
            psrd->accel_count++;
            if(psrd->method == sCurve ) //&& !psrd->bstopping
            {
                div1 = 2 * (int32_t)psrd->step_delay * (psrd->n2 + psrd->accel_count) + psrd->rest;
                div2 = (4 * psrd->accel_count + 1) * psrd->n2;
            }
            else
            {
                div1 = 2 * (int32_t)psrd->step_delay + psrd->rest;
                div2 = 4 * psrd->accel_count + 1;
            }
            new_step_delay = psrd->step_delay - div1 / div2;
            psrd->rest = div1 % div2;
            // Check if we at last step
            if(psrd->accel_count >= 0)
            {
                psrd->run_state = rsSTOP;
            }
        }
        else
        {
            while(1);
        }

        psrd->step_delay = new_step_delay;
        pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = new_step_delay - 1;
        pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = 0;
        pdma_buf->dma_buf[pdma_buf->dma_buf_len++] = new_step_delay/ 2;  
        
        if( (new_step_delay - 1) == 0 || new_step_delay/ 2 == 0)
        {
            while(1);
        }
        if(psrd->min_step_delay > (new_step_delay - 1))
            psrd->min_step_delay = new_step_delay - 1;
    }
    if(psrd->steps_request != HM_INVALID_STEP) psrd->steps_taken = step_count;
    pdma_buf->valid = 1;
}
void hm_tim_pause_resume(hm_step_info_t *psrd, bool bpause)
{
    if(bpause)
    {
        DMA_Cmd(psrd->pdma,  DISABLE);
        TIM_Cmd(psrd->ptim,  DISABLE);
        
        
    }
    else
    {
//        DMA_Cmd(psrd->pdma,  ENABLE);
        TIM_Cmd(psrd->ptim,  ENABLE);
        
    }
}
void hm_tim_start(hm_step_info_t *psrd, bool benable, uint64_t fre)
{
    if(benable)
    {
        psrd->ptim->CCR1  = psrd->step_delay - 1;
        psrd->ptim->ARR   = psrd->step_delay / 2;
        if(fre != 0) psrd->ptim->PSC   = (uint16_t) ((uint64_t)SystemCoreClock / fre) - 1;
        
 //       hm_motor_hw_enable_irq(psrd->idx, true);
        TIM_Cmd(psrd->ptim,  ENABLE);
        hm_led_chan(psrd->idx, true);
        hm_mark_axis_state(psrd->idx, asRuning);
        
        
    }
    else
    {
 //       hm_motor_hw_enable_irq(psrd->idx, false);
        TIM_Cmd(psrd->ptim,  DISABLE);
        hm_led_chan(psrd->idx, false);
        psrd->bdo_home = false;
        hm_mark_axis_state(psrd->idx, asStoped);
        
        HM_NET_PRINT("!stop(axis = %d)", psrd->idx);
    }
    
}
void hm_dma_setup(hm_step_info_t *psrd, bool benable)
{
    hm_dma_buf_info_t       *pdma_buf = &psrd->dma_buf[psrd->dma_buf_idx];
#if 0
    DMA_InitTypeDef         DMA_InitStructure;
    /*init DMA*/
    DMA_DeInit(psrd->pdma); /* DMA1 Channel5 is for TIM1 */
    DMA_InitStructure.DMA_PeripheralBaseAddr        = (uint32_t)&psrd->ptim->DMAR; 
    DMA_InitStructure.DMA_MemoryBaseAddr            = (uint32_t)pdma_buf->dma_buf; 
    DMA_InitStructure.DMA_DIR                       = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize                = pdma_buf->dma_buf_len;
    DMA_InitStructure.DMA_PeripheralInc             = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc                 = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize        = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize            = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode                      = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority                  = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M                       = DMA_M2M_Disable;
    DMA_Init(psrd->pdma, &DMA_InitStructure);  
    DMA_ITConfig(psrd->pdma, DMA_IT_TC | DMA_IT_HT, ENABLE);
 #endif
    
    psrd->pdma->CNDTR = pdma_buf->dma_buf_len;

/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
  /* Write to DMAy Channelx CMAR */
    psrd->pdma->CMAR = (uint32_t)pdma_buf->dma_buf;
  
    
 //   DMA_SetCurrDataCounter (psrd->pdma, 0);
    DMA_Cmd(psrd->pdma,  benable ? ENABLE : DISABLE);
}

void hm_dma_fill_next_buf(uint32_t axis)
{
    hm_step_info_t          *psrd           = &s_axis_info[axis];
    hm_dma_buf_info_t       *pdma_buf_next  = &psrd->dma_buf[(psrd->dma_buf_idx + 1) % HM_DMA_BUF_BLOCK_SZ];

    if(!pdma_buf_next->valid)
    {
        pdma_buf_next->dma_buf_len = 0;
        hm_dma_update(psrd, pdma_buf_next);
        psrd->dma_fill++;
    }

}

void hm_dma_ck(uint32_t axis)
{
    hm_step_info_t          *psrd           = &s_axis_info[axis];
    hm_dma_buf_info_t       *pdma_buf_cur   = &psrd->dma_buf[psrd->dma_buf_idx];
    hm_dma_buf_info_t       *pdma_buf_next  = &psrd->dma_buf[(psrd->dma_buf_idx + 1) % HM_DMA_BUF_BLOCK_SZ];
    uint16_t                step_dma        = pdma_buf_cur->dma_buf_len / HM_DMA_BUF_UNIT_SIZE;
    //update position
    if(psrd->dir == rdCW)
    {
        psrd->pos += step_dma;
    }
    else
    {
        psrd->pos -= step_dma;
    }
    pdma_buf_cur->valid = 0;
    
    psrd->steps_actual  += step_dma;
    
    if(psrd->steps_actual < psrd->steps_request || psrd->steps_request == HM_INVALID_STEP)
    {
        if(!pdma_buf_next->valid)
        {
            while(1);
        }
        psrd->dma_buf_idx = (psrd->dma_buf_idx + 1) % HM_DMA_BUF_BLOCK_SZ;
        hm_tim_pause_resume(psrd, true);
        hm_dma_setup(psrd, true);
        hm_tim_pause_resume(psrd, false);
    }
    else
    {
        psrd->bstopping         = false;
        hm_tim_start(psrd, false, 0);
        hm_dma_setup(psrd, false);
        
        
    }
    psrd->dma_irq_idx++;
}

void TIM1_UP_IRQHandler(void)
{
    uint32_t    aixs = 0;
    hm_step_info_t          *psrd           = &s_axis_info[aixs];
    psrd->steps_tim_irq++;
}

void DMA1_Channel5_IRQHandler(void)
{
    uint32_t    aixs = 0;
    if(DMA_GetITStatus(DMA1_IT_HT5))
    {
        hm_dma_fill_next_buf(aixs);
        DMA_ClearITPendingBit(DMA1_IT_HT5);
    }

    
    if(DMA_GetITStatus(DMA1_IT_TC5))
    {
        hm_dma_ck(aixs);
        DMA_ClearITPendingBit(DMA1_IT_TC5);
    }
  
}
void DMA1_Channel2_IRQHandler(void)
{
    uint32_t    aixs = 1;
    
    if(DMA_GetITStatus(DMA1_IT_HT2))
    {
        hm_dma_fill_next_buf(aixs);
        DMA_ClearITPendingBit(DMA1_IT_HT2);
    }    
    if(DMA_GetITStatus(DMA1_IT_TC2))
    {
        hm_dma_ck(aixs);
        DMA_ClearITPendingBit(DMA1_IT_TC2);
    }
}
void DMA1_Channel3_IRQHandler(void)
{
    uint32_t    aixs = 2;
    if(DMA_GetITStatus(DMA1_IT_HT3))
    {
        hm_dma_fill_next_buf(aixs);
        DMA_ClearITPendingBit(DMA1_IT_HT3);
    }    
    if(DMA_GetITStatus(DMA1_IT_TC3))
    {
        hm_dma_ck(aixs);
        DMA_ClearITPendingBit(DMA1_IT_TC3);
    }
}
void DMA2_Channel2_IRQHandler(void)
{
    uint32_t    aixs = 4;
    if(DMA_GetITStatus(DMA1_IT_HT2))
    {
        hm_dma_fill_next_buf(aixs);
        DMA_ClearITPendingBit(DMA1_IT_HT2);
    }    
    if(DMA_GetITStatus(DMA2_IT_TC2))
    {
        hm_dma_ck(aixs);
        DMA_ClearITPendingBit(DMA2_IT_TC2);
    }
}

void DMA1_Channel7_IRQHandler(void)
{
    uint32_t    aixs = 3;
    if(DMA_GetITStatus(DMA1_IT_HT7))
    {
        hm_dma_fill_next_buf(aixs);
        DMA_ClearITPendingBit(DMA1_IT_HT7);
    }    
    if(DMA_GetITStatus(DMA1_IT_TC7))
    {
        hm_dma_ck(aixs);
        DMA_ClearITPendingBit(DMA1_IT_TC7);
    }
}

static uint32_t mysqrt(uint32_t x)
{
    uint32_t xr;  // result register
    uint32_t q2;  // scan-bit register
    uint8_t f;   // flag (one bit)

    xr = 0;                     // clear result
    q2 = 0x40000000L;           // higest possible result bit
    do
    {
        if((xr + q2) <= x)
        {
            x -= xr + q2;
            f = 1;                  // set flag
        }
        else{
            f = 0;                  // clear flag
        }
        xr >>= 1;
        if(f){
            xr += q2;               // test flag
        }
    } while(q2 >>= 2);          // shift twice
    if(xr < x){
        return xr +1;             // add for rounding
    }
    else{
        return xr;
    }
}
RUN_STATE_E hm_step_motor_state(uint32_t axis)
{
    hm_step_info_t          *psrd           = &s_axis_info[axis];
    
    if(psrd->axis_state != asStoped)
    {
        return rsRUN;
    }
    else
    {
        return rsSTOP;
    }
}
AXIS_STATE_E hm_step_axis_state(uint32_t axis)
{
	  return s_axis_info[axis].axis_state;
}
uint32_t    hm_step_motor_last_actual_steps_moved(uint32_t axis)
{
    return s_axis_info[axis].steps_tim_irq;
}

bool        hm_step_motor_stop_smooth(uint32_t axis)
{
    /*recalculate the DMA buffer and do again*/
    hm_step_info_t          *psrd       = &s_axis_info[axis];
    int32_t                 acount      = 0;

    if(axis >= HM_MAX_AXIS) return false;
    if(hm_step_motor_state(axis) == rsSTOP || psrd->bstopping) 
    {
        HM_NET_PRINT("hm_step_motor_stop_smooth returned state = %d, bstoppoing= %d\n",
         hm_step_motor_state(axis), psrd->bstopping);
        return true;
    }
    
    acount = 100;  
    psrd->bstopping = true;
    psrd->rest      = 0;
    psrd->steps_request     = acount;
    psrd->steps_taken       = 0;
    psrd->steps_actual      = 0;  
      
   
    
    // make to decel stage
    psrd->run_state         = rsDECEL;    
   
    psrd->accel_count       = -acount;
   
    HM_NET_PRINT("hm_step_motor_stop_smooth(axis = %d, pos = %d acount = %d, rest = %d, psrd->run_state = %d, speed = %d)\n", 
        axis, psrd->pos, acount, psrd->rest, psrd->run_state, (int)(psrd->HM_STEP_FRE / psrd->step_delay));
    
    return true;
}

bool hm_step_motor_stop_sundden(uint32_t axis)
{
    hm_step_info_t          *psrd;
    hm_dma_buf_info_t       *pdma_buf_cur   = NULL;
    uint16_t                npassed         = 0;   
    if(axis >= HM_MAX_AXIS) return false;
    
    psrd       = &s_axis_info[axis];
    pdma_buf_cur   = &psrd->dma_buf[psrd->dma_buf_idx];
    
    psrd->bstopping         = false;
    
    hm_dma_setup(psrd, false);
    hm_tim_start(psrd, false, 0);
    npassed = (pdma_buf_cur->dma_buf_len - DMA_GetCurrDataCounter(psrd->pdma)) / HM_DMA_BUF_UNIT_SIZE;
    
    if(psrd->dir == rdCW) 
        psrd->pos += npassed;
    else 
        psrd->pos -= npassed; 
    HM_NET_PRINT("hm_step_motor_stop_sundden(axis = %d,pos = %d)\n",  axis, psrd->pos);
    
    return true;
}
uint32_t        hm_step_motor_get_curr_speed(uint32_t axis)
{
    hm_step_info_t          *psrd           = NULL;
    
    if(axis >= HM_MAX_AXIS) return 0xffffffff;
    
    psrd           = &s_axis_info[axis];
    if(hm_step_motor_state(axis) == rsSTOP) return 0;
    else return psrd->HM_STEP_FRE / (psrd->ptim->CCR1 * 2);
    
}
void        hm_step_motor_set_cur_pos(uint32_t axis, int32_t newpos)
{    
    if(axis >= HM_MAX_AXIS) return ;  
    s_axis_info[axis].pos = newpos;
}
void        hm_step_motor_set_home_stop_mode(uint32_t axis, bool blowtriger)
{
    if(axis >= HM_MAX_AXIS) return ;  
    HM_NET_PRINT("hm_step_motor_set_home_stop_mode(axis = %d, blowtriger = %d)\n",  axis, blowtriger);
    s_axis_info[axis].bhome_stop_low_triger = blowtriger;    
}
bool        hm_step_motor_get_home_stop_mode(uint32_t axis)
{
    if(axis >= HM_MAX_AXIS) return false;  
    return s_axis_info[axis].bhome_stop_low_triger;     
}
int32_t        hm_step_motor_get_cur_pos(uint32_t axis)
{
    hm_step_info_t          *psrd           = NULL;
    hm_dma_buf_info_t       *pdma_buf_cur   = NULL;
    uint16_t                npassed         = 0;
    
    if(axis >= HM_MAX_AXIS) return false;
    
    psrd           = &s_axis_info[axis];
    pdma_buf_cur   = &psrd->dma_buf[psrd->dma_buf_idx];
    
    if(hm_step_motor_state(axis) == rsSTOP) return psrd->pos;
    
    npassed = (pdma_buf_cur->dma_buf_len - DMA_GetCurrDataCounter(psrd->pdma)) / HM_DMA_BUF_UNIT_SIZE;
    
    return (psrd->dir == rdCW) ? (psrd->pos + npassed) : (psrd->pos - npassed);    
    
}
uint64_t tim_base_fre(uint32_t speed)
{
    if(speed <= 15) return 10000;
    else if(speed > 15 && speed <= 10000) return HM_STEP_FRE_INIT;
    else return HM_STEP_FRE_INIT;
    
}
void        hm_step_motor_set_axis_curve(uint32_t axis, methrd_e e)
{
    if(axis >= HM_MAX_AXIS || hm_step_motor_state(axis) != rsSTOP) 
    {
        HM_NET_PRINT("!!hm_step_motor_set_axis_curve return ,need stop axis = %d", axis);
        return;
    }
    s_axis_info[axis].method = e;
}
methrd_e    hm_step_motor_get_axis_curve(uint32_t axis)
{
    return s_axis_info[axis].method;
}
