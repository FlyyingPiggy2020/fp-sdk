/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : drv_err.h
 * @Author       : lxf
 * @Date         : 2024-12-05 15:29:19
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2024-12-05 15:33:06
 * @Brief        : 从岑老板的代码中抄袭
 */

#ifndef __DRV_ERR_H__
#define __DRV_ERR_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/

/*---------- macro ----------*/
#define DRV_ERR_OK         (0)
#define DRV_ERR_EOK        (0)
#define DRV_ERR_ERROR      (-1)
#define DRV_ERR_POINT_NONE (-2)
#define DRV_ERR_TIME_OUT   (-3)
#define DRV_ERR_BUSY       (-4)
#define DRV_ERR_NO_MEMORY  (-5)
#define DRV_ERR_WRONG_ARGS (-6)
#define DRV_ERR_WRONG_CRC  (-7)
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
