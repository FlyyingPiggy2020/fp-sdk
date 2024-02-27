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

bool pingpong_buffer_get_read_buf(struct pingpong_buffer *handler, void **pread_buf, fp_size_t *size)
{
    if (handler->count == 0) {
        return false;
    }

    *pread_buf = handler->buffer[handler->read_index];

    *size = handler->size[handler->read_index];
    return true;
}

void pingpong_buffer_set_read_done(struct pingpong_buffer *handler)
{
    if (handler->count == 0) {
        return;
    }

    DISABLE_IRQ();
    handler->read_index++;
    if (handler->read_index == 2) {
        handler->read_index = 0;
    }
    handler->count--;
    ENABLE_IRQ();
}

void pingpong_buffer_get_write_buf(struct pingpong_buffer *handler, void **pwrite_buf)
{
    *pwrite_buf = handler->buffer[handler->write_index];
}

void pingpong_buffer_set_write_done(struct pingpong_buffer *handler, fp_size_t size)
{
    DISABLE_IRQ();
    handler->size[handler->write_index] = size;
    handler->write_index++;
    if (handler->write_index == 2) {
        handler->write_index = 0;
    }
    handler->count++;
    ENABLE_IRQ();
}