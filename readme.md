# 目录

| 名称        | 备注                                                        |
| ----------- | ----------------------------------------------------------- |
| components  | 通用业务逻辑的组件                                          |
| driver      | 设备IO驱动                                                  |
| env_support | espidf环境支持（因为整体sdk重构过，所以不再适用，目前废弃） |
| Unity       | 单元测试组件 (因为整体sdk重构过，所以不再适用，目前废弃)    |
| utilities   | 一些实用的基础组件为整个sdk提供支持                         |

# TODO

- [ ] 使用struct命名结构体，而不是使用typedef struct。 符合posix的命名规则，避免造成命名空间的污染。



# 附录1：为什么不用typedef struct，typedef struct有什么隐患吗？

这是一个非常经典且有深度的问题，涉及到 **C语言工程规范**、**代码可读性** 以及 **Linux内核/RT-Thread的设计哲学**。

直接回答你的问题：**这种的写法在技术上是完全正确的，没有致命的逻辑错误。**

但是，在像 RT-Thread、Linux Kernel 这样的大型 C 语言项目中，大家倾向于不使用 `typedef` 来定义复杂的结构体，而是直接使用 `struct tag_name`。如果你的目标是融入这些开源生态，或者编写大型、复杂的驱动框架，你的写法存在以下几个**隐患**（或者说是工程上的劣势）：

### 1. 隐患一：互相包含（循环依赖）时的麻烦
这是最直接的技术痛点。在复杂的驱动框架中，结构体经常互相引用。

**场景**：假设 `can_device` 需要包含 `can_ops` 指针，而 `can_ops` 函数指针又要接收 `can_device` 指针。

**如果不使用 Typedef (官方推荐写法)：**
我们可以使用**前向声明 (Forward Declaration)**，非常干净：

```c
/* can.h */

// 1. 前向声明，告诉编译器 struct rt_can_device 存在
struct rt_can_device; 

// 2. 定义 ops，里面可以用 struct rt_can_device *
struct rt_can_ops {
    int (*send)(struct rt_can_device *can, void *buf);
};

// 3. 定义 device，里面包含 ops
struct rt_can_device {
    const struct rt_can_ops *ops;
};
```

**如果使用 Typedef (你的写法)：**
你需要非常小心定义的顺序，或者被迫混用：

```c
/* can.h */

typedef struct can_device_s can_device_t; // 必须先 typedef

typedef struct {
    // 这里必须用 can_device_t，如果此时它还没定义完，编译器会报错
    int (*send)(can_device_t *can, void *buf); 
} can_ops_t;

struct can_device_s { // 这里又要回头去写 struct 名字
    can_ops_t *ops;
};
```
**隐患**：当头文件包含关系复杂时，`typedef` 容易导致 "unknown type name" 错误，迫使你调整头文件引用顺序，增加了耦合度。而 `struct tag` 只需要加一行 `struct name;` 就能解决所有指针引用问题。

### 2. 隐患二：掩盖了“它是大块数据”的事实（性能陷阱）
这是 Linux 之父 Linus Torvalds 强烈反对在内核中使用 typedef 定义结构体的主要原因。

**你的写法：**
```c
void process_data(can_msg_t msg);
```
看到这行代码，程序员可能会潜意识觉得 `can_msg_t` 像是一个 `int` 或者 `handle`（句柄），传递它的开销很小。但实际上它有 20 多个字节，直接传值会产生栈拷贝，性能较差。

**官方写法：**
```c
void process_data(struct rt_can_msg msg);
```
当程序员看到 `struct` 关键字时，脑子里的警报会响：“**这是一个结构体！**”。如果它很大，我应该传指针 `struct rt_can_msg *msg`，而不是传值。

**隐患**：`typedef` 隐藏了数据类型的大小属性，容易诱导使用者写出低效的代码（例如由值传递导致的无谓拷贝）。

### 3. 隐患三：命名空间污染与 POSIX 标准冲突
在标准 C 和 POSIX 标准中，以 `_t` 结尾的类型通常保留给系统使用（如 `size_t`, `time_t`, `uint32_t`）。

虽然在嵌入式裸机开发中这不是大问题，但在操作系统层面上：
*   你的 `can_msg_t` 可能会和系统未来的某个类型重名。
*   `typedef` 把名字直接引入了全局命名空间。而 `struct can_msg` 的名字只在 `struct` 标签的命名空间里，更加安全。

### 4. 什么时候该用 Typedef，什么时候不用？

在 RT-Thread 和 Linux 的设计哲学中，有一条清晰的分界线：

*   **完全不透明的对象 (Opaque Objects) -> 用 Typedef**
    *   例如：`rt_thread_t`、`rt_device_t`、`rt_sem_t`。
    *   **原因**：用户不需要（也不应该）知道里面的成员变量。用户拿到的本质上是一个**指针**（句柄）。
    *   RTT 源码：`typedef struct rt_thread *rt_thread_t;` （注意这里 typedef 的是指针）。

*   **透明的数据容器 (Transparent Data) -> 用 struct**
    *   例如：`struct rt_can_msg`、`struct sockaddr`。
    *   **原因**：用户必须访问里面的成员（如 `msg.id`, `msg.data`）。这是数据的载体，而不是对象的句柄。

### 总结建议

1.  **如果只是自己玩或小项目**：你的 `typedef` 写法完全没问题，看着清爽，写着少敲键盘。
2.  **如果是设计驱动框架或给 RTT 贡献代码**：
    *   对于**数据报文**（如 `can_msg`），请使用 `struct rt_can_msg`。
    *   这不仅是为了符合编码规范，更是为了让代码在复杂的包含关系中更健壮，并提醒使用者注意内存拷贝的开销。

所以，在你的 `can_device.h` 设计中，建议去掉 `typedef`，保留 `struct rt_can_msg` 的写法，这样显得更“专业”，更符合系统级开发的惯例。
