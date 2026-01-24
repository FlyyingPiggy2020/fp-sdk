/**
 * @file common/pingpong_buffer/pingpong_buffer.c
 *
 * Copyright (C) 2021
 *
 * pingpong_buffer.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @author HinsShum hinsshum@qq.com
 *
 * @encoding utf-8
 */

/*---------- includes ----------*/
#include "options.h"
#include "pingpong.h"
#include "string.h"
/*---------- macro ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
/*---------- variable ----------*/
/*---------- function ----------*/
void pingpong_buffer_init(struct pingpong_buffer *handler, void *buf0, void *buf1)
{
    memset(handler, 0, sizeof(*handler));
    handler->buffer[0] = buf0;
    handler->buffer[1] = buf1;
}

bool pingpong_buffer_get_read_buf(struct pingpong_buffer *handler, void **pread_buf)
{
    if (handler->read_avaliable[0]) {
        handler->read_index = 0;
    } else if (handler->read_avaliable[1]) {
        handler->read_index = 1;
    } else {
        return false;
    }
    *pread_buf = handler->buffer[handler->read_index];
    // 标记该缓冲正在被读取
    handler->read_active[handler->read_index] = true;
    return true;
}

void pingpong_buffer_set_read_done(struct pingpong_buffer *handler)
{
    handler->read_avaliable[handler->read_index] = false;
    handler->read_active[handler->read_index] = false; // 清除正在读取标记
}

bool pingpong_buffer_get_write_buf(struct pingpong_buffer *handler, void **pwrite_buf)
{
    // 优先检查当前write_index指向的缓冲是否可读
    if (!handler->read_avaliable[handler->write_index]) {
        // 当前缓冲不可读，可以安全写入
        *pwrite_buf = handler->buffer[handler->write_index];
        return true;
    }

    // 当前缓冲可读，检查另一个缓冲
    uint8_t other_index = !handler->write_index;
    if (!handler->read_avaliable[other_index]) {
        // 另一个缓冲不可读，切换到该缓冲写入
        handler->write_index = other_index;
        *pwrite_buf = handler->buffer[handler->write_index];
        return true;
    }

    // 两个缓冲都可读（都满），检查是否有缓冲正在被读取
    if (!handler->read_active[0]) {
        // buffer[0]未激活读取，覆盖buffer[0]
        handler->write_index = 0;
        *pwrite_buf = handler->buffer[0];
        return false; // 返回false，表示数据被覆盖
    } else if (!handler->read_active[1]) {
        // buffer[1]未激活读取，覆盖buffer[1]
        handler->write_index = 1;
        *pwrite_buf = handler->buffer[1];
        return false; // 返回false，表示数据被覆盖
    } else {
        // 两个缓冲都在被读取，无法安全写入(理论上调用合理不可能进入这里)
        xlog_error("ping pong buff error.\n");
        return false; // 返回false，无法获取缓冲
    }
}

void pingpong_buffer_set_write_done(struct pingpong_buffer *handler)
{
    handler->read_avaliable[handler->write_index] = true;
    handler->write_index = !handler->write_index;
}
