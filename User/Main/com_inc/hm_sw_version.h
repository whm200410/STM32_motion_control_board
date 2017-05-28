#ifndef __HM_SW_VERSION_H
#define __HM_SW_VERSION_H

#define SW_VER_MAJOR    2
#define SW_VER_MINOR    7

/*
1.2 增加了以太网的debug支持，若需要开通debug支持，只需要发送hmDebugOpe命令即可
1.4 版本增加了在中断中检查限位并停止的功能,双重检查
1.5 版本修复了1.4中的hm_motor_limit_logical_check问题
1.6 电机开启后开限位中断，停止后关限位中断
1.8 main的时候就检测是否有网络连接，没有的话闪error灯，直到连上
1.9 print增加了clock
2.0 用axis 的AXIS_STATE_E判断是否停止，不然的话可能会中途dma中断的时候tim停止了，但是确实在运行
2.1 修改了hm_motor_limit_logical_check
2.2 gpio 输入加入了延时功能，判断
2.3 handler_extern_irq中把hm_step_motor_state(axis) == rsRUN改为了 (psrd->axis_state == asRuning),防止在开始run的时候，设置dma的时候出发中断
2.4 加入了resetboard
2.5 测试
2.6 增加了曲线加减速
2.7 缓停算法改进，其他的算法改进，修复bug
*/
#define SW_VERSION      (SW_VER_MAJOR << 8 | SW_VER_MINOR)

#endif
