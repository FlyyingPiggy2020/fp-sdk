/*
 * Copyright (c) 2025 by Lu Xianfan.
 * @FilePath     : fpevent.h
 * @Author       : lxf
 * @Date         : 2025-05-09 09:19:24
 * @LastEditors  : lxf_zjnb@qq.com
 * @LastEditTime : 2025-05-09 09:19:26
 * @Brief        :
 */

#ifndef __FPEVENT_H__
#define __FPEVENT_H__
#ifdef __cplusplus
extern "C" {
#endif
/*---------- includes ----------*/
/*---------- macro ----------*/
#define EVENT_SET_BIT(flag, mask)          ((flag) |= (mask))  // 将mask位置1
#define EVENT_CLEAR_BIT(flag, mask)        ((flag) &= ~(mask)) // 将mask位置0
#define EVENT_TOGGLE_BIT(flag, mask)       ((flag) ^= (mask))  // 反转mask位

#define EVENT_IS_AND_BIT_SET(flag, mask)   (((flag) & (mask)) == (mask)) // 满足所有位才是1
#define EVENT_IS_OR_BIT_SET(flag, mask)    ((flag) & (mask))             // 满足1个就是1
#define EVENT_IS_AND_BIT_CLEAR(flag, mask) (((flag) & (mask)) == 0)      // 所有位都是0才满足
#define EVENT_IS_OR_BIT_CLEAR(flag, mask)  (~(flag) & (mask))            // 任意一位是0就满足

/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/
#ifdef __cplusplus
}
#endif
#endif
