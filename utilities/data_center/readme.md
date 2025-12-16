# 数据中心组件

本目录提供了一个基于账号系统的数据处理和管理组件，支持数据的订阅和发布机制。

## 文件结构

```
data_center/
├── demo/                  # 数据中心使用示例
│   ├── data_proc.c        # 数据处理实现
│   ├── data_proc.h        # 数据处理头文件
│   ├── data_proc_def.h    # 数据处理定义
│   ├── dp_ble.c           # BLE数据处理实现
│   ├── dp_list.inc        # 数据处理列表定义
│   └── test_data_center.c # 数据中心测试代码
├── account.c              # 账号管理实现
├── account.h              # 账号管理头文件
├── data_center.c          # 数据中心核心实现
├── data_center.h          # 数据中心头文件
└── readme.md              # 本说明文件
```

## 核心功能

- 基于账号的数据订阅/发布机制
- 支持多账号管理
- 支持账号间的相互订阅
- 数据处理和分发
- 可扩展的数据处理链

## 核心组件说明

### 1. data_center.h

**功能**：数据中心的头文件，包含所有公共API声明

**主要内容**：
- 数据中心结构体定义
- 数据中心操作函数声明
- 账号管理相关函数声明

### 2. data_center.c

**功能**：数据中心的实现文件，包含所有算法实现

**主要内容**：
- 数据中心初始化和销毁
- 账号添加和删除
- 账号搜索和管理

### 3. account.h

**功能**：账号管理的头文件，包含账号相关API声明

**主要内容**：
- 账号结构体定义
- 账号订阅和发布函数声明
- 数据处理函数声明

### 4. account.c

**功能**：账号管理的实现文件

**主要内容**：
- 账号初始化和销毁
- 订阅和取消订阅操作
- 数据发布和接收

## 基本使用方法

### 1. 初始化数据中心

```c
#include "data_center.h"

// 创建数据中心
data_center_t center = data_center_init("my_data_center");
```

### 2. 创建并添加账号

```c
// 创建账号
account_t account = account_init("my_account");

// 将账号添加到数据中心
data_center_add_account(center, account);
```

### 3. 订阅和发布数据

```c
// 账号A订阅账号B
account_subscribe(account_a, account_b);

// 账号B发布数据
uint8_t data[] = {0x01, 0x02, 0x03};
account_publish(account_b, data, sizeof(data));

// 账号A接收数据（通过回调函数）
static void data_receive_callback(account_t from, const void *data, size_t len) {
    printf("Received data from %s: ", account_get_name(from));
    for (size_t i = 0; i < len; i++) {
        printf("%02X ", ((uint8_t *)data)[i]);
    }
    printf("\n");
}

// 设置数据接收回调
account_set_receive_callback(account_a, data_receive_callback);
```

### 4. 销毁资源

```c
// 从数据中心移除账号
data_center_remove_account(center, account);

// 销毁账号
account_deinit(account);

// 销毁数据中心
data_center_deinit(center);
```

## 账号系统工作原理

1. **账号（Account）**：数据的生产者和消费者，每个账号都有唯一的名称
2. **订阅关系**：账号可以订阅其他账号，形成数据流向
3. **数据发布**：当账号发布数据时，所有订阅该账号的账号都会收到数据
4. **主账号**：数据中心自带一个主账号，默认订阅所有添加到数据中心的账号

## 应用场景

- 多模块数据通信
- 事件驱动系统
- 数据流处理
- 传感器数据分发
- 实时监控系统

## 数据中心与账号关系

```
Data Center
├── Main Account (自动订阅所有账号)
└── Account Pool
    ├── Account A
    ├── Account B
    └── Account C
```

## 扩展功能

### 1. 数据处理链

```c
// 添加数据处理函数
static void data_process_function(const void *data_in, size_t len_in, void **data_out, size_t *len_out) {
    // 数据处理逻辑
    // ...
}

// 为账号添加数据处理函数
account_add_data_processor(account, data_process_function);
```

### 2. 多数据中心

```c
// 创建多个数据中心
data_center_t center1 = data_center_init("center1");
data_center_t center2 = data_center_init("center2");

// 在不同数据中心间移动账号
data_center_remove_account(center1, account);
data_center_add_account(center2, account);
```

## 使用示例

详细的使用示例请参考`demo/`目录下的代码，特别是`test_data_center.c`文件，展示了数据中心的完整使用流程。