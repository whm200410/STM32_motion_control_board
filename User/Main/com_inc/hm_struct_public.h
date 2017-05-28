#ifndef __HM_STRUCT_PUBLIC_H
#define __HM_STRUCT_PUBLIC_H

/*是否停止状态，是否碰到了限位，或者是否使能*/
enum hmAxisStatus_e
{
    hmsStoped   = 0x01,
    hmsHitCW    = 0x02,
    hmsHitCCW   = 0x04,
    hmsHitHome  = 0x08,
    hmsEnabled  = 0x10,
};

/*各个限位停止模式急停还是缓停*/

enum hmStopMode_e
{
    hmtSmonthStop,
    hmtSuddentStop,
};
enum hmCmdGetAxisSetting_e
{
    hmsHomeTriger   = 0x01,
    hmsStopModeCW   = 0x02,
    hmsStopModeCCW  = 0x04,
    hmsStopModeHome = 0x08,
    hmsCurve        = 0x10,
};

/*原点触发方式是下降沿触发还是低电平触发*/
enum hmHomeTriggerMode_e
{
    hmtFallTrigger,
    hmtLowTrigger
};

/**/
enum hmCurveMode_e
{
    hmtSmode,
    hmtTMode
};

/*获得轴的状态*/
typedef struct hmCmdGetAxisStatus_s
{
    uint32_t axis;                  //轴编号
    int32_t  pos;                   //轴位置
    uint32_t axis_status;           //hmAxisStatus_e 中|
    uint32_t speed;                 //当前速度
}hmCmdGetAxisStatuse_t;


/*获得或者设置当前轴的情况*/
typedef struct hmCmdGetAxisSetting_s
{
    uint32_t  axis;                 //轴编号
    uint32_t  bHomeTriger;          /*原点触发方式是下降沿触发还是低电平触发*/
    uint32_t  bStopModeCW;          /*各个限位停止模式急停还是缓停*/
    uint32_t  bStopModeCCW;
    uint32_t  bStopModeHome;   
    uint8_t   flag;
    uint8_t   bget;                 //get or set
    uint8_t   Tcurve;
}hmCmdGetAxisSetting_t;

#endif
