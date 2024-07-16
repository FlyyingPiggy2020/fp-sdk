#include "unity.h"
#include "fp_soft_timer.h"

// 测试用例初始化函数
void setUp(void) {
    // 初始化代码
}

// 测试用例清理函数
void tearDown(void) {
    // 清理代码
}

// 测试 fp_timer_create 函数
void test_fp_timer_create(void) {
    fp_timer_t *timer;
    uint32_t period = 100;
    void *user_data = NULL;

    timer = fp_timer_create(NULL, period, user_data);

    // 断言检查 timer 是否被正确创建
    TEST_ASSERT_NOT_NULL(timer);
    TEST_ASSERT_EQUAL_UINT32(period, timer->period);
    TEST_ASSERT_EQUAL_PTR(user_data, timer->user_data);
}

void test_fp_timer_handler(void) {
    uint32_t ret = fp_timer_handler();
    // 断言检测 timer handler是否执行成功
    TEST_ASSERT_EQUAL_UINT32(0, ret);
}
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fp_timer_create);
    RUN_TEST(test_fp_timer_handler);
    return UNITY_END();
}