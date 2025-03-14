# stm32-washer-demo
- 项目描述：基于 STM32F103C8T6 的洗衣机小项目，供初学练习。
- 演示视频：https://www.bilibili.com/video/BV19bCVYkEFB/
- Github链接：https://github.com/KenneLu/stm32-washer-demo
- Gitee链接：https://gitee.com/KenneLu/stm32-washer-demo



## 分支版本

- v1.0.0分支：裸机。
- v2.0.0分支：裸机 + 面向对象。
- v3.0.0分支：FreeRTOS + 面向对象。
-  main 分支：FreeRTOS + 面向对象。



## 实物图

- **注：若图无法展示，可以在本地【README.assets】中找到原图。**
- 通过**旋转编码器**来控制上下，短按为启动键，长按为退出键。另有一个**按钮开关**作为待机键。
- 通过**LED灯**和**蜂鸣器**指示状态，**OLED**显示详情。
- 通过**温湿度计**获取温度，判断水温，判断烘干湿度。
- 通过**直流电机**模拟滚筒洗衣机，**舵机**模拟洗衣机门锁。
- 通过**对射式红外传感器**获取距离，判断洗衣机门是否打开。
- 通过**陀螺仪加速度计**获取姿态，判断洗衣机是否发生震动和倾斜。
- 通过**EEPROM**存储当前运行状态，意外断电恢复后，复原上次工作状态。

![](https://github.com/KenneLu/stm32-washer-demo/blob/main/README.assets/Img2.jpg)

![](\README.assets\Img2.jpg)



## 原理图

- 下图外设均为模块，**请忽略模块的电路细节**，主要参考与STM32的接口连线。

![](https://github.com/KenneLu/stm32-washer-demo/blob/main/README.assets/Img1.jpg)

![](\README.assets\Img1.jpg)



## 状态机

- 下图为状态机。
- **注意：v3.0.0以后改用FreeRTOS，部分状态机会被拆分成Task。**

![](https://github.com/KenneLu/stm32-washer-demo/blob/main/README.assets/Img3.jpg)

![](\README.assets\Img3.jpg)



## FreeRTOS 任务图

- 下图为任务图。
- **注意：仅v3.0.0及以上版本适用。**
- Task_Washer_Run 核心逻辑依然是洗衣机的状态机。相比于 v2.0.0分支版本，把【按键扫描】【安全监测】【急停/暂停/继续】【OLED显示】【数据管理】从状态机里拆分出来，作为单独的Task运行，以提升响应速度和并发处理。

![](https://github.com/KenneLu/stm32-washer-demo/blob/main/README.assets/Img4.jpg)

![](\README.assets\Img4.jpg)



## 参考资料

- **硬件驱动：  @江协科技 —《STM32入门教程-2023版》**
- **按键检测：  @无际单片机编程 —《单片机物联网模块化编程》**
- **多级菜单：  @加油哦大灰狼 —《多级菜单快速移植框架》**    
- **状态机：      @码农爱学习 —《STM32状态机编程实例》**    
- **FreeRTOS:  @韦东山 — FreeRTOS系列专题**

