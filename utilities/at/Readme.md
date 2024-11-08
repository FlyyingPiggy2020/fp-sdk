# 注意事项

## 查询的实现方式

里面的实现方式并不是一个个遍历。而是先进行分组，例如BLE打头的分成一组，然后MQTT打头的分成另外一组。
去搜寻对应的AT指令的时候，会先找到时哪个组，再去找对应的子命令。

所以分组的部分不能重叠，例如一个组是BLE，另外一个组是BLEA，我是没有办法做分辨的。

因为BLE里面的子命令，比如BLESETNAME，和BLEASETNAME，我没办法区分到底哪个是子命令哪个是组。

# 注册示例

ld文件中添加：

```
    /* at command */
    _at_cmd_start = .;
    KEEP(*(.at_cmd.))
    _at_cmd_end = .;
```

```c
static at_cmd_struct at_ble_cmd_list[] = {
    { "AT+BLEMAC", at_ble_mac },
    { "AT+BLENAME", NULL },
};

AT_CMD_SET_REGISTER_FN(at_ble_cmd_list, ble, "AT+BLE");
```
