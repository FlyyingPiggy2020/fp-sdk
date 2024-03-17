`components`组件

`driver`驱动程序

`device `实际的硬件设备，一个设备可以对应多个驱动程序

`utilities`公共库

`protocol`协议


# 使用说明

esp32 把fp-sdk目录放到对应工程components下,在最外层的CmakeList.txt里面增加`add_definitions(-DUSE_ESP=1)`