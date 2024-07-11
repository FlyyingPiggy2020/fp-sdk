双向链表的C语言实现，代码不是我写的，很经典的代码，网上抄的，下面是这个文件提供的api：

* LIST_HEAD_INIT(name)：使用给定名称初始化链表头结构。
* LIST_HEAD(name)：声明并初始化具有给定名称的链表头结构。
* list_entry(ptr, type, member)：获取给定链表项的结构体。
* list_first_entry(ptr, type, member)：获取链表的第一个元素。
* list_last_entry(ptr, type, member)：获取链表的最后一个元素。
* list_first_entry_or_null(ptr, type, member)：获取链表的第一个元素，如果链表为空则返回 NULL。
* list_next_entry(pos, type, member)：获取链表中的下一个元素。
* list_prev_entry(pos, type, member)：获取链表中的上一个元素。
* list_for_each(pos, head)：遍历链表。
* list_for_each_prev(pos, head)：反向遍历链表。
* list_for_each_safe(pos, n, head)：安全地遍历链表（支持删除链表项）。
* list_for_each_prev_safe(pos, n, head)：安全地反向遍历链表。
* list_for_each_entry(pos, type, head, member)：遍历给定类型的链表。
* list_for_each_entry_reverse(pos, type, head, member)：以反向顺序遍历给定类型的链表。
* list_prepare_entry(pos, type, head, member)：为在 list_for_each_entry_continue() 中使用的元素准备位置。
* list_for_each_entry_continue(pos, type, head, member)：继续遍历给定类型的链表。
* list_for_each_entry_continue_reverse(pos, type, head, member)：以反向顺序继续遍历给定类型的链表。
* list_for_each_entry_from(pos, type, head, member)：从当前位置开始遍历给定类型的链表。
* list_for_each_entry_safe(pos, n, type, head, member)：安全地遍历给定类型的链表（支持删除链表项）。
* list_for_each_entry_safe_continue(pos, n, type, head, member)：安全地继续遍历链表（支持删除链表项）。
* list_for_each_entry_safe_from(pos, n, type, head, member)：从当前位置安全地遍历给定类型的链表（支持删除链表项）。
* list_for_each_entry_safe_reverse(pos, n, type, head, member)：以反向顺序安全地遍历给定类型的链表（支持删除链表项）。
* list_safe_reset_next(pos, n, type, member)：重置陈旧的 list_for_each_entry_safe 循环。
