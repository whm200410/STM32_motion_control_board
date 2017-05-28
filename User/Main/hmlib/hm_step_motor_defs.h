/************************************************************************
 $Revision: 55 $                                                           
************************************************************************/

#ifndef __HM_STEP_MOTOR_DEFS_H
#define __HM_STEP_MOTOR_DEFS_H
#include "stm32f10x.h"
#include "hm_step_motor_hw_config.h"
typedef enum 
{
    rsSTOP, 
    rsACCEL,
    rsDECEL,
    rsRUN
} RUN_STATE_E;

typedef enum 
{
    asPreparing, 
    asStoped,
    asRuning,
} AXIS_STATE_E;

typedef enum 
{
    rdCW,
    rdCCW
}RUN_DIR_E;

typedef enum 
{    
    TCurve,
    sCurve,
}methrd_e;

#define        HM_DMA_BUF_SIZE                    100
#define        HM_DMA_BUF_UNIT_SIZE               3
#define        HM_STEP_FRE_INIT                   1000000   
//#define        HM_STEP_FRE_DIV_10000_MUL_0676     (HM_STEP_FRE / 10000) *0.676
#define        HM_DMA_BUF_BLOCK_SZ                2
#define        HM_STOP_ACCEL_VALUE                200000
#define        HM_INVALID_STEP                    ((uint32_t)-1)
#define        HM_SENSE_HIT                       0
typedef struct hm_dma_buf_info_s
{
    uint16_t    dma_buf[HM_DMA_BUF_UNIT_SIZE * HM_DMA_BUF_SIZE];
    uint16_t    dma_buf_len;   
    uint16_t    valid;
}hm_dma_buf_info_t;

typedef struct hm_step_info_s
{
    uint64_t            HM_STEP_FRE;
    //!for HW resource
    hm_step_motor_hw_config_t  motor_hw_config;
    
    methrd_e            method;
    GPIO_TypeDef        *port_pul;
    TIM_TypeDef         *ptim;
    DMA_Channel_TypeDef *pdma;
    uint32_t            remap;
    uint32_t            nhw_ch;
    //! What part of the speed ramp we are in.
    RUN_STATE_E run_state ;
    //! Direction stepper motor should move.
    RUN_DIR_E   dir ;
    volatile AXIS_STATE_E    axis_state;
    
    
    //!start for stopping smoothly
    bool        bstopping;
    bool        bdo_home;
    bool        bhome_stop_low_triger;    //true for low lever triger or false for fall from high to low lever triger
    
    uint32_t    n2;
    //!how many stepped needed for stop
    uint32_t    steps_to_stop;
    
    //!the aceel for smooth stop
    uint32_t    stop_accel;
    
    uint32_t    idx;
    //!total steps request by up caller, if for keep running mode, this should be -1
    uint32_t    steps_request;
    uint32_t    steps_taken;
    uint32_t    steps_actual;
    uint32_t    steps_tim_irq;
    uint32_t    dma_irq_idx;
    //!current postion hold by me
    int32_t     pos;
    //! What step_pos to start acceleration
    uint32_t    accel_steps;
    //! What step_pos to start decelaration, if for keep running mode, this should be -1
    uint32_t    decel_start;    
    //! Sets deceleration rate.
    int32_t     decel_val;
    //! Counter used when accelerateing/decelerateing to calculate step_delay.
    int32_t     accel_count;    
    
    uint32_t    rest;
    uint16_t    last_accel_delay;
    //! Minimum time delay (max speed)
    uint16_t    min_delay;    
     //! Peroid of next timer delay. At start this value set the accelration rate.
    uint16_t    step_delay; 
    uint16_t    min_step_delay; 

    //!for HW config
    uint16_t    pul_pin;
    uint16_t    rcc_mod1;  
    uint16_t    rcc_mod2;
    uint16_t    rcc_mod;
    uint16_t    irq;
    uint16_t    dma_fill;
    
    //!dma data used to count
    hm_dma_buf_info_t   dma_buf[HM_DMA_BUF_BLOCK_SZ]; 
    uint16_t            dma_buf_idx;
}hm_step_info_t;
#endif
