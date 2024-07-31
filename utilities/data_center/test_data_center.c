// Unity Test Framework Header
#include "unity.h"

// 被测试的文件头文件
#include "data_center.h"
#include "account.h"
#include "demo/data_proc.h"
#include "string.h"

#define ACCOUNT_SEND_CMD(account, CMD) \
do{ \
    dp_##account##_info_t info; \
    DATA_PROC_INIT_STRUCT(info); \
    info.cmd = CMD; \
	data_center_t *center = data_proc_get_center(); \
	account_notify_from_id(center->account_main, #account, &info, sizeof(dp_##account##_info_t));\
}while(0)

// 全局变量
data_center_t *test_center = NULL;
void setUp(void) {
}

void tearDown(void) {
}

void test_data_proc_init(void) {
    printf("====================data_proc_init====================\n");
    data_proc_init();
    ACCOUNT_SEND_CMD(ble, DP_BLE_START);
    ACCOUNT_SEND_CMD(ble, DP_BLE_STOP);
    do {
        dp_ble_info_t info;
        data_center_t *center = data_proc_get_center();
        memset(&info,0,sizeof(dp_ble_info_t));
        account_pull_from_id(center->account_main, "ble", &info, sizeof(dp_ble_info_t));
        printf("ble name: %s\n", info.name);
    } while (0);
    
}

int main(void) {
    // 启动测试
    UNITY_BEGIN();
    RUN_TEST(test_data_proc_init);
    return UNITY_END();
}