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
 * @FilePath     : list.h
 * @Author       : lxf
 * @Date         : 2023-12-20 10:55:34
 * @LastEditors  : FlyyingPiggy2020 154562451@qq.com
 * @LastEditTime : 2023-12-20 10:57:01
 * @Brief        :
 */

/*---------- includes ----------*/
/*---------- macro ----------*/
/*---------- type define ----------*/
/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/*---------- end of file ----------*/

#ifndef __LIST_HEAD_H
#define __LIST_HEAD_H

#ifdef __cplusplus
extern "C" {
#endif

/*---------- includes ----------*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*---------- macro ----------*/

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))
#elif defined(__GNUC__)
#define container_of(ptr, type, member)                      \
    ({                                                       \
        const __typeof(((type *)0)->member) *__mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member));   \
    })
#elif defined(_MSC_VER)
#define container_of(ptr, type, member) ((type *)((char *)ptr - offsetof(type, member)))
#else
#define container_of(ptr, type, member) ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))
#endif

/* initialize list head
 */
#define LIST_HEAD_INIT(name) \
    {                        \
        &(name), &(name)     \
    }
#define LIST_HEAD(name)                                               struct list_head name = LIST_HEAD_INIT(name)

/**
 * @brief Get the struct for this entry.
 * @param ptr: the &struct list_head pointer.
 * @param type: the type of the struct this is embeded in.
 * @param member: the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member)                                 container_of(ptr, type, member)

/**
 * @brief Get the first element from a list.
 * @note That list is expected to be not empty.
 * @param ptr: the &struct list_head pointer.
 * @param type: the type of the struct this is embeded in.
 * @param member: the name of the list_struct within the struct.
 */
#define list_first_entry(ptr, type, member)                           list_entry((ptr)->next, type, member)

/**
 * @brief Get the last element from a list.
 * @note That list is expected to be not empty.
 * @param ptr: the &struct list_head pointer.
 * @param type: the type of the struct this is embeded in.
 * @param member: the name of the list_struct within the struct.
 */
#define list_last_entry(ptr, type, member)                            list_entry((ptr)->prev, type, member)

/**
 * @brief Get the first element from a list.
 * @note That if the list is empty, it returns NULL.
 * @param ptr: the &struct list_head pointer.
 * @param type: the type of the struct this is embeded in.
 * @param member: the name of the list_struct within the struct.
 */
#define list_first_entry_or_null(ptr, type, member)                   (!list_empty(ptr) ? list_first_entry(ptr, type, member) : NULL)

/**
 * @brief Get the next element in list.
 * @param pos: the type * to cursor.
 * @param type: the type of the struct this is embeded in.
 * @param member: the name of the list_struct within the struct.
 */
#define list_next_entry(pos, type, member)                            list_entry((pos)->member.next, type, member)

/**
 * @brief Get the prev element in list.
 * @param pos: the type * to cursor.
 * @param type: the type of the struct this is embeded in.
 * @param member: the name of the list_struct within the struct.
 */
#define list_prev_entry(pos, type, member)                            list_entry((pos)->member.prev, type, member)

/**
 * @brief iterate over a list.
 * @param pos: the $struct list_head to use as a loop cursor.
 * @param head: the head for your list.
 */
#define list_for_each(pos, head)                                      for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief iterate over a list backword.
 * @param pos: the $struct list_head to use as a loop cursor.
 * @param head: the head for your list.
 */
#define list_for_each_prev(pos, head)                                 for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * @brief iterate over a list safe against removal of list
 * entry.
 * @param pos: the $struct list_head to use as a loop cursor.
 * @param n: another $struct list_head to use as temporary storage.
 * @param head: the head for your list.
 */
#define list_for_each_safe(pos, n, head)                              for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * @brief iterate over a list backword safe against removal
 * of list entry.
 * @param pos: the $struct list_head to use as a loop cursor.
 * @param n: another $struct list_head to use as temporary storage.
 * @param head: the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head)                         for (pos = (head)->prev, n = pos->prev; pos != (head); pos = n, n = pos->prev)

/**
 * @brief Iterate over list of given type.
 * @param pos: the type * to use as a loop cursor.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, type, head, member)                  for (pos = list_first_entry(head, type, member); &pos->member != (head); pos = list_next_entry(pos, type, member))

/**
 * @brief Iterate backwords over list of given type.
 * @param pos: the type * to use as a loop cursor.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, type, head, member)          for (pos = list_last_entry(head, type, member); &pos->member != (head); pos = list_prev_entry(pos, type, member))

/**
 * @brief Prepare a pos entry for use in list_for_each_entry_continue().
 * @param pos: the type * to use as a start point.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_prepare_entry(pos, type, head, member)                   ((pos) ?: list_entry(head, type, member))

/**
 * @brief Continue iteration over list of given type.
 * @param pos: the type * to use as a loop cursor.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_continue(pos, type, head, member)         for (pos = list_next_entry(pos, type, member); &pos->member != (head); pos = list_next_entry(pos, type, member))

/**
 * @brief Iterate backwords from the given point.
 * @param pos: the type * to use as a loop cursor.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_continue_reverse(pos, type, head, member) for (pos = list_prev_entry(pos, type, member); &pos->member != (head); pos = list_prev_entry(pos, type, member))

/**
 * @brief Iterate over list of given type from the current point.
 * @param pos: the type * to use as a loop cursor.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_from(pos, type, head, member)             for (; &pos->member != (head); pos = list_next_entry(pos, type, member))

/**
 * @brief Iterate over list of given type safe against removal
 * of list entry.
 * @param pos: the type * to use as a loop cursor.
 * @param n: another type * to use as temporary storage.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, type, head, member) \
    for (pos = list_first_entry(head, type, member), n = list_next_entry(pos, type, member); &pos->member != (head); pos = n, n = list_next_entry(pos, type, member))

/**
 * @brief Continue list iteration safe against removal.
 * @param pos: the type * to use as a loop cursor.
 * @param n: another type * to use as temporary storage.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe_continue(pos, n, type, head, member) \
    for (pos = list_next_entry(pos, type, member), n = list_next_entry(pos, type, member); &pos->member != (head); pos = n, n = list_next_entry(pos, type, member))

/**
 * @brief Iterate over list from current point safe against removal.
 * @param pos: the type * to use as a loop cursor.
 * @param n: another type * to use as temporary storage.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe_from(pos, n, type, head, member) for (n = list_next_entry(pos, type, member); &pos->member != (head); pos = n, n = list_next_entry(pos, type, member))

/**
 * @brief Iterate backwords over list safe against removal.
 * @param pos: the type * to use as a loop cursor.
 * @param n: another type * to use as temporary storage.
 * @param type: the type of the struct this is embeded in.
 * @param head: the head for your list.
 * @param member: the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe_reverse(pos, n, type, head, member) \
    for (pos = list_last_entry(head, type, member), n = list_prev_entry(pos, type, member); &pos->member != (head); pos = n, n = list_prev_entry(pos, type, member))

/**
 * @brief Reset a stale list_for_each_entry_safe loop.
 * @note list_safe_reset_next is not safe to use in general if the
 * list may be modified concurrently (eg. the lock is dropped in
 * the loop body). An exception to this is if the cursor element
 * (pos) is pinned in the list, and list_safe_reset_next is called
 * afrer re-taking the lock and before completing the current iteration
 * of the loop body.
 * @param pos: the loop cursor used in the list_for_each_entry_safe loop.
 * @param n: temporary storage used in list_for_each_entry_safe.
 * @param memebr: the name of the list_struct within the struct.
 */
#define list_safe_reset_next(pos, n, type, member) n = list_next_entry(pos, type, member)

/*---------- type define ----------*/
struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

/*---------- variable prototype ----------*/
/*---------- function prototype ----------*/
/**
 * @brief Initialize the linked list to an empty linked list.
 * @param list: list head will be initialize.
 * @retval None
 */
static inline void INIT_LIST_HEAD(struct list_head *list)
{
    list->next = list;
    list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *pnew, struct list_head *prev, struct list_head *next)
{
    pnew->next = next;
    pnew->prev = prev;
    next->prev = pnew;
    prev->next = pnew;
}

/**
 * @brief Add a new entry. Insert a new entry after the specified
 * head. This is good for implementing stacks.
 * @param pnew: new entry to be added.
 * @param head: list head to add it after.
 * @retval None
 */
static inline void list_add(struct list_head *pnew, struct list_head *head)
{
    __list_add(pnew, head, head->next);
}

/**
 * @brief Add a new entry. Insert a new entry before the specified
 * head. This is useful for implementing queues.
 * @param pnew: new entry to be added.
 * @param head: list head to add it before.
 * @retval None
 */
static inline void list_add_tail(struct list_head *pnew, struct list_head *head)
{
    __list_add(pnew, head->prev, head);
}

/*
 * Delete a list entry by marking the prev/next entries point to
 * each other.
 *
 * This is only for internal list manipulation where we know the
 * prev/next entries already!
 */
static inline void __list_del(struct list_head *prev, struct list_head *next)
{
    prev->next = next;
    next->prev = prev;
}

/**
 * @brief Deletes entry from list.
 * @note list_empty() on entry does not return true after this, the
 * entry is in an undefined state.
 * @param entry: the element to delete from the list.
 * @retval None
 */
static inline void list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->prev = NULL;
    entry->next = NULL;
}

/**
 * @brief Deletes entry from list and reinitialize it.
 * @param entry: the element to delete from the list.
 * @retval None
 */
static inline void list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

/**
 * @brief Replace old entry by new one. If @old was empty, it
 * will be overwritten.
 * @param old: The elemnet to be replaced.
 * @param pnew: The new element to insert.
 * @retval None
 */
static inline void list_replace(struct list_head *old, struct list_head *pnew)
{
    pnew->next = old->next;
    pnew->next->prev = pnew;
    pnew->prev = old->prev;
    pnew->prev->next = pnew;
}

/**
 * @brief Replace old entry by new one. If @old was empty, it
 * will be overwritten.
 * @param old: The elemnet to be replaced.
 * @param pnew: The new element to insert.
 * @retval None
 */
static inline void list_replace_init(struct list_head *old, struct list_head *pnew)
{
    list_replace(old, pnew);
    INIT_LIST_HEAD(old);
}

/**
 * @brief Delete from one list and add as another's head.
 * @param list: the entry to move.
 * @param head: the head that will precede our entry.
 * @retval None
 */
static inline void list_move(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add(list, head);
}

/**
 * @brief Delete from one list and add as another's tail
 * @param list: the entry to move.
 * @param head: the head that will folliw our entry.
 * @retval None
 */
static inline void list_move_tail(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

/**
 * @brief Tests whether @list is the last entry in list @head.
 * @param list: the entry to test.
 * @param head: the head of the list.
 * @retval true: @list is the last entry in the list @head.
 *         false: @list is not the last entry in the list @head.
 */
static inline bool list_is_last(const struct list_head *list, const struct list_head *head)
{
    return (list->next == head);
}

/**
 * @brief Test whether a list is empty.
 * @param head: the list to test.
 * @retval true: @head is an empty list.
 *         false: @head is not an empty list.
 */
static inline bool list_empty(const struct list_head *head)
{
    return (head->next == head);
}

/**
 * @brief Rotate the list to the left.
 * @param head: the head of the list.
 * @retval None
 */
static inline void list_rotate_left(struct list_head *head)
{
    struct list_head *first = NULL;

    if (!list_empty(head)) {
        first = head->next;
        list_move_tail(first, head);
    }
}

/**
 * @brief Tests whether a list is empty and not being modified.
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU
 * might be in the process of modifying either member(next or prev).
 * @note Using list_empty_careful() without synchronization can
 * only be safe if the only activity that can happen to the list
 * entry is list_del_init(). Eg. it can not be used if another
 * CPU could re-list_add() it.
 * @param head: the list to test.
 * @retval true: @head is an empty list and no other CPU is modifying
 * the member(next or prev).
 *         false: @head is not an empty list or other CPU is modifying
 * the member(next or prev).
 */
static inline bool list_empty_careful(const struct list_head *head)
{
    struct list_head *next = head->next;

    return ((next == head) && (next == head->prev));
}

/**
 * @brief Tests whether a list has just one entry.
 * @param head: the list to test.
 * @retval true: @head has just one entry.
 *         false: @head not has just one entry.
 */
static inline bool list_is_singular(const struct list_head *head)
{
    return (!list_empty(head) && (head->next == head->prev));
}

static inline void __list_cut_position(struct list_head *list, struct list_head *head, struct list_head *entry)
{
    struct list_head *new_first = entry->next;

    list->next = head->next;
    list->next->prev = list;
    list->prev = entry;
    entry->next = list;
    head->next = new_first;
    new_first->prev = head;
}

/**
 * @brief Cut a list into two. This helper moves the initial
 * part of @head, up to and including @entry, from @head to
 * @list. You should pass an @entry an element you know is on
 * @head. @list should be an empty list or a list you do not
 * care about losing its data.
 * @param list: a new list to add all removed entries.
 * @param head: a list with entries.
 * @param entry: an entry within head, could be the head itself
 * and if so we won't cut the list.
 * @retval None
 */
static inline void list_cut_position(struct list_head *list, struct list_head *head, struct list_head *entry)
{
    do {
        if (list_empty(head)) {
            break;
        }
        if (list_is_singular(head) && (head->next != entry && head != entry)) {
            break;
        }
        if (entry == head) {
            INIT_LIST_HEAD(list);
        } else {
            __list_cut_position(list, head, entry);
        }
    } while (0);
}

static inline void __list_splice(const struct list_head *list, struct list_head *prev, struct list_head *next)
{
    struct list_head *first = list->next;
    struct list_head *last = list->prev;

    first->prev = prev;
    prev->next = first;
    last->next = next;
    next->prev = last;
}

/**
 * @brief Join two lists, this is designed for stacks.
 * @param list: the new list to add.
 * @param head: the place to add it in the first list.
 * @retval None
 */
static inline void list_splice(const struct list_head *list, struct list_head *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head, head->next);
    }
}

/**
 * @brief Join two lists, each list being queue.
 * @param list: the new list to add.
 * @param head: the place to add it in thr first list.
 * @retval None
 */
static inline void list_splice_tail(const struct list_head *list, struct list_head *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head->prev, head);
    }
}

/**
 * @brief Join two lists and reinitialise the emptied list.
 * The list at @list is reinitialise.
 * @param list: the new list to add.
 * @param head: the place to add it in the first list.
 * @retval None
 */
static inline void list_splice_init(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head, head->next);
        INIT_LIST_HEAD(list);
    }
}

/**
 * @brief Join two lists and reinitialise the emptied list. Each of
 * the lists is a queue. The list at @list is reinitialise.
 * @param list: the new list to add.
 * @param head: the place to add it in the first list.
 * @retval None
 */
static inline void list_splice_tail_init(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head->prev, head);
        INIT_LIST_HEAD(list);
    }
}

#ifdef __cplusplus
}
#endif
#endif /* __LIST_HEAD_H */
