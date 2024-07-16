gcc -o test_fp_soft_timer test_fp_soft_timer.c fp_soft_timer.c ../../Unity/src/unity.c -I../../Unity/src
./test_fp_soft_timer
rm -f test_fp_soft_timer