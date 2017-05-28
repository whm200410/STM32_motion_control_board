This was a project long time ago, it did not demostrate my current ablility!!!

It uses STM32F107 to implement a 4 axis stepper motor control, it supports both trapiziodal and s curve profile.
Four stepper motors can do position and free run independently, up to 1MHz pulse/second is supported.
STM32F107 is selected as MCU to do all the controls, it receives commond from PC or other source by ethenet, instruct specific motor to move.
One MCU controls 4 stepper motor, up to 8 board can be used simutanously, so a 32 maximum stepper motor can be controlled at the same time.
It uses PWM and DMA to do motion profile calculation at runtime.

It lacks of comment in the source code.
