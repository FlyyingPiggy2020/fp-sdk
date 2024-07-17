# if you want to enable LOG_TRACE
# gcc -DFP_LOG_TRACE_TIMER -o test_fp_soft_timer test_fp_soft_timer.c fp_soft_timer.c ../../Unity/src/unity.c -I../../Unity/src
gcc -o test_fp_soft_timer test_fp_soft_timer.c fp_soft_timer.c ../../Unity/src/unity.c -I../../Unity/src
./test_fp_soft_timer
rm -f test_fp_soft_timer