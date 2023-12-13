/*MIT License

Copyright (c) 2023 Lu Xianfan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
 * Copyright (c) 2023 by Moorgen Tech. Co, Ltd.
 * @FilePath     : tlv.c
 * @Author       : lxf
 * @Date         : 2023-12-08 10:34:58
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-08 10:35:19
 * @Brief        : tlv format package
 */

/*---------- includes ----------*/

#include "tlv.h"
/*---------- macro ----------*/

#define TLV_LENGTH_MAX 32
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/

/**
 * @brief 根据type类型，在tlv报文里查找对应的tag并返回TLV
 * @param {uint8_t} *data tlv报文
 * @param {uint16_t} length 报文长度
 * @param {uint8_t} tag 想要寻找的tag
 * @return {*}
 */
TLV findTLV(const uint8_t *data, uint16_t length, uint8_t tag)
{
    uint16_t index = 0;
    TLV emptyTLV;

    while (index < length) {
        TLV tlv;
        tlv.tag = data[index++];
        tlv.length = data[index++];
        if (tlv.length > TLV_LENGTH_MAX) {
            dp_printf("[error][findTLV] tlv.lenth %d is bigger than max size.\r\n", tlv.length);
            goto end;
        }
        tlv.value = (uint8_t *)malloc(tlv.length);
        // 将值复制到分配的内存中
        memcpy(tlv.value, &data[index], tlv.length);
        // 如果找到匹配的标签，则返回对应的TLV元素
        if (tlv.tag == tag) {
            return tlv;
        }
        // 更新索引
        index = index + tlv.length;
        // 释放值的内存
        free(tlv.value);
    }
    emptyTLV.tag = 0;
    emptyTLV.length = 0;
    emptyTLV.value = NULL;
    return emptyTLV;
end:
    emptyTLV.tag = 0;
    emptyTLV.length = 0;
    emptyTLV.value = NULL;
    return emptyTLV;
}
/*---------- end of file ----------*/
