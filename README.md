# stm32-washer-demo
- 基于 STM32F103C8T6 的洗衣机小项目，供初学练习。



## 实物图

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

![](https://github.com/KenneLu/stm32-washer-demo/blob/main/README.assets/Img3.jpg)

![](\README.assets\Img3.jpg)