/**
 * @file heap.c
 *
 * Copyright (C) 2020
 *
 * heap.c is free software: you can redistribute it and/or modify
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

/*---------- includes ----------*/
#include "heap.h"
#include <stdlib.h>
#ifdef CONFIG_HEAP_FILE
#include CONFIG_HEAP_FILE
#endif

/*---------- macro ----------*/
/* heap protect callback defition */
#ifndef CONFIG_HEAP_LOCK
#define CONFIG_HEAP_LOCK()
#endif

#ifndef CONFIG_HEAP_UNLOCK
#define CONFIG_HEAP_UNLOCK()
#endif

/* heap port macro defition */
#ifndef CONFIG_HEAP_BYTE_ALIGNMENT
#define CONFIG_HEAP_BYTE_ALIGNMENT      (8)
#endif

#define HEAP_BYTE_ALIGNMENT_MASK        (CONFIG_HEAP_BYTE_ALIGNMENT - 1)
#if (HEAP_BYTE_ALIGNMENT_MASK & CONFIG_HEAP_BYTE_ALIGNMENT) || (CONFIG_HEAP_BYTE_ALIGNMENT > 32)
#error "CONFIG_HEAP_BYTE_ALIGNMENT must be one of the following values: 1, 2, 4, 8, 16 or 32"
#endif

#ifndef CONFIG_HEAP_TOTAL_SIZE
#error "Please define the head total size before use this heap program"
#endif

/* Assumes 8bit bytes! */
#define HEAP_BITS_PER_BYTE              ((size_t)8)
/* Block sizes must not get too small. */
#define HEAP_MINIMUM_BLOCK_SIZE         ((size_t)(heap_struct_size << 1))

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- type define ----------*/
typedef struct a_block_link {
    struct a_block_link *pnext_free_block;  /*<< The next free block in the list. */
    size_t block_size;                      /*<< The size of the free block. */
} block_link_t;

/*---------- variable ----------*/
/* Allocate the memory for the heap. */
static uint8_t heap[CONFIG_HEAP_TOTAL_SIZE];
/* The size of the structure placed at the beginning of each allocated memory
 * block must by correctly byte aligned. */
static const size_t heap_struct_size = (sizeof(block_link_t) + ((size_t)(CONFIG_HEAP_BYTE_ALIGNMENT - 1))) & ~((size_t)HEAP_BYTE_ALIGNMENT_MASK);
/* Create a couple of list links to mark the start and end of the list. */
static block_link_t start, *pend = NULL;
/* Keeps track of the number of calls to allocate and free memory as well as the
 * number of free bytes remaining, but says nothing about fragmentation. */
static size_t free_bytes_remaining = 0U;
static size_t minimum_free_bytes_remaining = 0U;
/* Gets set to the top bit of an size_t type.  When this bit in the xBlockSize
 * member of an BlockLink_t structure is set then the block belongs to the
 * application.  When the bit is free the block is still part of the free heap
 * space. */
static size_t block_allocate_bit = 0;

/*---------- function ----------*/
static void heap_init(void)
{
    block_link_t *pblock_first_free = NULL;
    uint8_t *paligned_heap = NULL;
    size_t address = 0;
    size_t total_heap_size = CONFIG_HEAP_TOTAL_SIZE;

    address = (size_t)heap;
    if(0 != (address & HEAP_BYTE_ALIGNMENT_MASK)) {
        address += (CONFIG_HEAP_BYTE_ALIGNMENT - 1);
        address &= ~((size_t)HEAP_BYTE_ALIGNMENT_MASK);
        total_heap_size -= address - (size_t)heap;
    }
    paligned_heap = (uint8_t *)address;
    /* xStart is used to hold a pointer to the first item in the list of free
     * blocks.  The void cast is used to prevent compiler warnings. */
    start.pnext_free_block = (void *)paligned_heap;
    start.block_size = (size_t)0;
    /* pxEnd is used to mark the end of the list of free blocks and is inserted
     * at the end of the heap space. */
    address = ((size_t)paligned_heap) + total_heap_size;
    address -= heap_struct_size;
    address &= ~((size_t)HEAP_BYTE_ALIGNMENT_MASK);
    pend = (void *)address;
    pend->block_size = 0;
    pend->pnext_free_block = NULL;
    /* To start with there is a single free block that is sized to take up the
     * entire heap space, minus the space taken by pxEnd. */
    pblock_first_free = (void *)paligned_heap;
    pblock_first_free->block_size = address - (size_t)pblock_first_free;
    pblock_first_free->pnext_free_block = pend;
    /* Only one block exists - and it covers the entire usable heap space. */
    free_bytes_remaining = pblock_first_free->block_size;
    minimum_free_bytes_remaining = pblock_first_free->block_size;
    /* Work out the position of the top bit in a size_t variable. */
    block_allocate_bit = ((size_t)1) << ((sizeof(size_t) * HEAP_BITS_PER_BYTE) - 1);
}

static void heap_insert_block_into_free_list(block_link_t *pblock_insert)
{
    block_link_t *pinterator = NULL;
    uint8_t *p = NULL;

    for(pinterator = &start; pinterator->pnext_free_block < pblock_insert; 
        pinterator = pinterator->pnext_free_block) {
    }
    p = (uint8_t *)pinterator;
    if((p + pinterator->block_size) == (uint8_t *)pblock_insert) {
        pinterator->block_size += pblock_insert->block_size;
        pblock_insert = pinterator;
    }
    p = (uint8_t *)pblock_insert;
    if((p + pblock_insert->block_size) == (uint8_t *)pinterator->pnext_free_block) {
        if(pinterator->pnext_free_block != pend) {
            pblock_insert->block_size += pinterator->pnext_free_block->block_size;
            pblock_insert->pnext_free_block = pinterator->pnext_free_block->pnext_free_block;
        } else {
            pblock_insert->pnext_free_block = pend;
        }
    } else {
        pblock_insert->pnext_free_block = pinterator->pnext_free_block;
    }
    if(pinterator != pblock_insert) {
        pinterator->pnext_free_block = pblock_insert;
    }
}

void *heap_malloc(size_t wanted_size)
{
    block_link_t *pblock = NULL, *pblock_previous = NULL, *pblock_new = NULL;
    void *preturn = NULL;

    CONFIG_HEAP_LOCK();
    {
        if(NULL == pend) {
            heap_init();
        }
        /* Check the requested block size is not so large that the top bit is
         * set.  The top bit of the block size member of the BlockLink_t structure
         * is used to determine who owns the block - the application or the
         * kernel, so it must be free. */
        if(0 == (wanted_size & block_allocate_bit)) {
            if(0 < wanted_size) {
                wanted_size += heap_struct_size;
                /* Ensure that blocks are always aligned to the required number
                 * of bytes. */
                if(0x00 != (wanted_size & HEAP_BYTE_ALIGNMENT_MASK)) {
                    wanted_size += (CONFIG_HEAP_BYTE_ALIGNMENT - (wanted_size & HEAP_BYTE_ALIGNMENT_MASK));
                }
            }
            if((0 < wanted_size) && (wanted_size <= free_bytes_remaining)) {
                /* Traverse the list from the start	(lowest address) block until
                 * one	of adequate size is found. */
                pblock_previous = &start;
                pblock = start.pnext_free_block;
                while((pblock->block_size < wanted_size) &&
                      (pblock->pnext_free_block != NULL)) {
                    pblock_previous = pblock;
                    pblock = pblock->pnext_free_block;
                }
                if(pblock != pend) {
                    preturn = (void *)(((uint8_t *)pblock_previous->pnext_free_block) + heap_struct_size);
                    pblock_previous->pnext_free_block = pblock->pnext_free_block;
                    /* If the block is larger than required it can be split into
                     * two. */
                    if((pblock->block_size - wanted_size) > HEAP_MINIMUM_BLOCK_SIZE) {
                        pblock_new = (void *)(((uint8_t *)pblock) + wanted_size);
                        pblock_new->block_size = pblock->block_size - wanted_size;
                        pblock->block_size = wanted_size;
                        heap_insert_block_into_free_list(pblock_new);
                    }
                    free_bytes_remaining -= pblock->block_size;
                    if(minimum_free_bytes_remaining > free_bytes_remaining) {
                        minimum_free_bytes_remaining = free_bytes_remaining;
                    }
                    pblock->block_size |= block_allocate_bit;
                    pblock->pnext_free_block = NULL;
                }
            }
        }
    }
    CONFIG_HEAP_UNLOCK();

    return preturn;
}

void heap_free(void *pfree)
{
    uint8_t *p = (uint8_t *)pfree;
    block_link_t *plink = NULL;

    if(NULL != pfree) {
        p -= heap_struct_size;
        plink = (void *)p;
        if(0 != (plink->block_size & block_allocate_bit)) {
            if(NULL == plink->pnext_free_block) {
                /* The block is being returned to the heap - it is no longer
                 * allocated. */
                plink->block_size &= ~block_allocate_bit;
                CONFIG_HEAP_LOCK();
                {
                    /* Add this block to the list of free blocks. */
                    free_bytes_remaining += plink->block_size;
                    heap_insert_block_into_free_list(plink);
                }
                CONFIG_HEAP_UNLOCK();
            }
        }
    }
}

size_t heap_get_free_remaining_size(void)
{
    return free_bytes_remaining;
}

size_t heap_get_minimum_free_remaining_size(void)
{
    return minimum_free_bytes_remaining;
}
