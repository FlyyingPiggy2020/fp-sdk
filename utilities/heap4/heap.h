/**
 * @file heap.h
 *
 * Copyright (C) 2020
 *
 * heap.h is free software: you can redistribute it and/or modify
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
 * @note reference from FreeRTOS heap_4.c
 */
#ifndef __HEAP_H
#define __HEAP_H

/*---------- includes ----------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
extern void *heap_malloc(size_t wanted_size);
extern void heap_free(void *pfree);
extern size_t heap_get_free_remaining_size(void);
extern size_t heap_get_minimum_free_remaining_size(void);

#endif /* __HEAP_H */
