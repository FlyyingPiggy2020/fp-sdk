支持4个优先级：`INIT_BOARD_EXPORT INIT_DEVICE_EXPORT ``INIT_COMPONENT_EXPORT INIT_APP_EXPORT`

其中BOARD优先级最高，APP优先级最低。

注意：所有硬件初始化部分都在BOARD里面完成。
例如串口初始化。

而类似于log组件这种初始化必须放在串口初始化后面。所以log初始化用DEVICE或者让COMPONET。

一定要注意你的程序里面的组件，是否对初始化顺序有要求。

先初始化的，用优先级高的。
