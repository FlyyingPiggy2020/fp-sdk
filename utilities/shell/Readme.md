# Shell命令行组件

本目录提供了一个轻量级的命令行交互组件，用于嵌入式系统的调试和控制。

## 文件结构

```
shell/
├── cmds/                 # 命令实现目录
│   ├── Readme.md         # 命令开发说明
│   ├── shell_cmd_clear.c # 清屏命令实现
│   └── shell_cmd_clear1.c # 另一个清屏命令实现
├── inc/                  # 头文件目录
│   └── shell.h           # Shell主头文件
├── Readme.md             # 本说明文件
├── shell.c               # Shell主实现
├── shell.lf              # 词法分析器定义
├── shell_executor.c      # 命令执行器
├── shell_parser.c        # 命令解析器
└── shell_prompt.c        # 命令提示符实现
```

## 核心功能

- 命令行解析和执行
- 命令自动补全
- 历史命令记录
- 自定义命令扩展
- 支持管道和重定向（部分功能）

## 核心组件说明

### 1. shell.h

**功能**：Shell组件的主头文件，包含所有公共API声明

**主要内容**：
- Shell初始化和配置结构体
- 命令注册和执行函数
- 输入输出函数声明

### 2. shell.c

**功能**：Shell组件的主实现，协调各个子模块

**主要内容**：
- Shell初始化和配置
- 主循环实现
- 输入处理

### 3. shell_parser.c

**功能**：命令解析器，将输入的命令字符串解析为可执行的命令结构

**主要内容**：
- 命令分词
- 参数解析
- 语法检查

### 4. shell_executor.c

**功能**：命令执行器，负责查找并执行注册的命令

**主要内容**：
- 命令查找
- 参数传递
- 执行结果处理

### 5. shell_prompt.c

**功能**：命令提示符实现，负责显示和更新命令提示符

**主要内容**：
- 提示符格式定义
- 提示符更新

## 基本使用方法

### 1. 初始化Shell

```c
#include "shell.h"

// 定义Shell配置
static shell_config_t shell_config = {
    .prompt = "$ ",
    .history_size = 10,
    .echo = true
};

// 初始化Shell
void shell_init(void) {
    shell_create(&shell_config);
}
```

### 2. 处理输入

```c
// 在主循环中处理Shell输入
void main_loop(void) {
    while (1) {
        // 获取字符输入
        char ch = getchar();
        if (ch != 0) {
            // 将字符输入传递给Shell
            shell_input(ch);
        }
        // 处理其他任务
    }
}
```

### 3. 注册自定义命令

```c
// 自定义命令处理函数
static int my_command_handler(int argc, char **argv) {
    printf("My command executed with %d arguments:\n", argc - 1);
    for (int i = 1; i < argc; i++) {
        printf("Argument %d: %s\n", i, argv[i]);
    }
    return 0;
}

// 注册命令
void register_my_commands(void) {
    shell_register_command("mycmd", my_command_handler, "My custom command");
}
```

### 4. 运行Shell

```c
int main(void) {
    // 初始化硬件和系统
    system_init();
    
    // 初始化Shell
    shell_init();
    
    // 注册自定义命令
    register_my_commands();
    
    // 启动主循环
    main_loop();
    
    return 0;
}
```

## 内置命令

| 命令 | 功能 | 说明 |
| ---- | ---- | ---- |
| clear | 清屏 | 清除终端屏幕内容 |

## 自定义命令开发

### 命令处理函数格式

```c
int command_handler(int argc, char **argv) {
    // 命令处理逻辑
    return 0; // 返回0表示成功，非0表示失败
}
```

### 命令注册

```c
shell_register_command(
    "command_name",     // 命令名称
    command_handler,    // 命令处理函数
    "Command description" // 命令描述
);
```

### 命令示例：打印系统信息

```c
static int cmd_info_handler(int argc, char **argv) {
    printf("System Information:\n");
    printf("- SDK Version: 1.0.0\n");
    printf("- Platform: Embedded System\n");
    printf("- Clock Speed: 100 MHz\n");
    return 0;
}

// 注册命令
shell_register_command("info", cmd_info_handler, "Show system information");
```

## 高级功能

### 1. 命令参数解析

```c
static int cmd_set_handler(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <parameter> <value>\n", argv[0]);
        return -1;
    }
    
    char *param = argv[1];
    int value = atoi(argv[2]);
    
    // 设置参数逻辑
    printf("Set %s to %d\n", param, value);
    return 0;
}
```

### 2. 命令自动补全

```c
// 为命令添加自动补全功能
static char **cmd_file_completion(int argc, char **argv) {
    static char *files[] = {"file1.txt", "file2.txt", "file3.txt", NULL};
    return files;
}

// 注册带自动补全的命令
shell_register_command_with_completion(
    "cat", 
    cmd_cat_handler, 
    "Show file content",
    cmd_file_completion
);
```

## 配置选项

| 配置项 | 类型 | 说明 | 默认值 |
| ------ | ---- | ---- | ------ |
| prompt | char* | 命令提示符字符串 | "$ " |
| history_size | int | 历史命令记录大小 | 10 |
| echo | bool | 是否回显输入字符 | true |
| max_line_length | int | 最大命令行长度 | 128 |
| max_args | int | 最大命令参数数量 | 10 |

## 使用建议

1. 为Shell分配足够的内存，特别是历史命令和输入缓冲区
2. 自定义命令名称应简洁明了，避免与内置命令冲突
3. 命令处理函数应具有良好的错误处理能力
4. 在资源受限的系统上，可以适当减少历史命令数量和缓冲区大小
5. 为复杂命令提供详细的帮助信息

## 扩展开发

- 可以通过修改`shell.lf`文件扩展词法分析器
- 可以通过实现自定义的输入输出函数支持不同的终端设备
- 可以添加管道和重定向功能以增强Shell能力