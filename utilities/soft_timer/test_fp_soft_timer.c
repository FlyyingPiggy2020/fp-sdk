#include "unity.h"
#include "fp_soft_timer.h"
    fp_timer_t *timer1;
    fp_timer_t *timer2;
// 测试用例初始化函数
void setUp(void) {
    timer1 = fp_timer_create(NULL,100,NULL);
    timer2 = fp_timer_create(NULL,50,NULL);
}

// 测试用例清理函数
void tearDown(void) {
    fp_timer_del(timer1);
    fp_timer_del(timer2);
}

// 测试正常删除定时器
void test_fp_tiemr_del_normal(void)
{
    uint32_t ret;
    _fp_timer_core_init();
    fp_timer_del(timer2);
    ret = fp_timer_handler();
    TEST_ASSERT_EQUAL_UINT32(100, ret);
}

// 测试删除不存在的定时器
void test_fp_timer_del_non_existent(void) {
    fp_timer_t timer;
    int result = fp_timer_del(&timer);
    TEST_ASSERT_NOT_EQUAL(true, result);
}


void test_fp_timer_handler(void) {
    _fp_timer_core_init();
    uint32_t ret = fp_timer_handler();
    // 断言检测 timer handler是否执行成功
    TEST_ASSERT_EQUAL_UINT32(50, ret);
}
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fp_timer_handler);
    RUN_TEST(test_fp_tiemr_del_normal);
    RUN_TEST(test_fp_timer_del_non_existent);
    return UNITY_END();
}