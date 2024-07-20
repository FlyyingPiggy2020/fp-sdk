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
#include "inc/pingpong.h"
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
    return true;
}

void pingpong_buffer_set_read_done(struct pingpong_buffer *handler)
{
    handler->read_avaliable[handler->read_index] = false;
}

void pingpong_buffer_get_write_buf(struct pingpong_buffer *handler, void **pwrite_buf)
{
    if (handler->write_index == handler->read_index) {
        handler->write_index = !handler->read_index;
    }
    *pwrite_buf = handler->buffer[handler->write_index];
}

void pingpong_buffer_set_write_done(struct pingpong_buffer *handler)
{
    handler->read_avaliable[handler->write_index] = true;
    handler->write_index = !handler->write_index;
}
