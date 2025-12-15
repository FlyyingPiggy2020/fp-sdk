/*
 * Copyright (c) 2024 by Lu Xianfan.
 * @FilePath     : errorno.h
 * @Author       : lxf
 * @Date         : 2024-12-05 15:29:19
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-08-22 16:17:46
 * @Brief        : 从岑老板的代码中抄袭
 */

#ifndef __DRV_ERR_H__
#define __DRV_ERR_H__

#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/

/*---------- macro ----------*/
#define E_OK         (0)
#define E_ERROR      (-1)
#define E_POINT_NONE (-2)
#define E_TIME_OUT   (-3)
#define E_BUSY       (-4)
#define E_NO_MEMORY  (-5)
#define E_WRONG_ARGS (-6)
#define E_WRONG_CRC  (-7)
#define E_FULL       (-8)
#define E_EMPTY      (-9)
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
