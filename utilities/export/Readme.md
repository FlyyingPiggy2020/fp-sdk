# 更新日志

20250426 ：1.重构，增加POLL EXPORT功能

20241026 ：1.增加对博流芯片的支持

20240711 ：1.增加export模块



# 移植说明

1.每毫秒调用一次`fp_tick_inc`

2.`main`函数的最后调用`fp_run`



# API说明

参考export.h的注释

