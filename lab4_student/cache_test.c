#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#define ARRAY_SIZE (1 << 30)                                    // test array size is 2^28

typedef unsigned char BYTE;										// define BYTE as one-byte type

BYTE array[ARRAY_SIZE];											// test array
const int L2_cache_size = 1 << 18;

const int test_time = 1 << 16;

double get_usec(const struct timeval tp0, const struct timeval tp1)
{
    return 1000000 * (tp1.tv_sec - tp0.tv_sec) + tp1.tv_usec - tp0.tv_usec;
}

// have an access to arrays with L2 Data Cache'size to clear the L1 cache
void Clear_L1_Cache()
{
    memset(array, 0, L2_cache_size);
}

// have an access to arrays with ARRAY_SIZE to clear the L2 cache
void Clear_L2_Cache()
{
    memset(array, 0, ARRAY_SIZE);
}

void Test_Cache_Size()
{
    printf("**************************************************************\n");
    printf("Cache Size Test\n");

    for (int offset = 10; offset < 20; offset++) {
        unsigned long bound = 1 << offset;
        // 将数组划分为四块进行访问（因为整段数组进行访存结果不明显）
        BYTE* part_0 = (BYTE*) malloc(bound >> 2);
        BYTE* part_1 = (BYTE*) malloc(bound >> 2);
        BYTE* part_2 = (BYTE*) malloc(bound >> 2);
        BYTE* part_3 = (BYTE*) malloc(bound >> 2);
        // 用于随机地址访存
        register unsigned long mod = bound >> 2;
        // 记录访存时间
        struct timeval tp[2];
        // 测试次数
        register unsigned long cache_test_time = 1 << 15;
        // 消除L2Cache的影响
        Clear_L2_Cache();

        /*
         * 测试段
         */
        gettimeofday(&tp[0], NULL);
        for (register unsigned long i = 0; i < cache_test_time; i++) {
            // 随机地址
            register unsigned long index = rand() % mod;
            part_0[index] = i;
            part_1[index] = i;
            part_2[index] = i;
            part_3[index] = i;
        }
        gettimeofday(&tp[1], NULL);

        // 输出数据
        printf("[Test_Array_Size = %-5ldKB]\t Average access time: %.3lf us\n", (bound >> 10), get_usec(tp[0], tp[1]));
        free(part_0);
        free(part_1);
        free(part_2);
        free(part_3);
    }
}

void Test_L1C_Block_Size()
{
    printf("**************************************************************\n");
    printf("L1 DCache Block Size Test\n");

    Clear_L1_Cache();											// Clear L1 Cache

    for (unsigned long offset = 4; offset < 10; offset++) {
        // 申请测试数组空间
        BYTE* test_array = (BYTE*) malloc(ARRAY_SIZE);
        // 数组访问间隔
        unsigned long jump = 1u << offset;
        // 刷新L1Cache
        Clear_L1_Cache();
        // 记录访存时间
        struct timeval tp[2];
        // 测试次数
        register unsigned long block_test_time = 1 << 15;
        // 数组起始下标
        register unsigned long index = 0;

        /*
         * 测试段
         */
        gettimeofday(&tp[0], NULL);
        for (register unsigned long i = 0; i < block_test_time; i++) {
            test_array[index] = i;
            index = (index + jump) % ARRAY_SIZE;
        }
        gettimeofday(&tp[1], NULL);

        // 输出数据
        printf("[Test_Array_Jump = %-3ldB]\t Average access time: %3f us\n", jump, get_usec(tp[0], tp[1]));
        free(test_array);
    }
}

void Test_L2C_Block_Size()
{
    printf("**************************************************************\n");
    printf("L2 Cache Block Size Test\n");

    Clear_L2_Cache();											// Clear L2 Cache

    // TODO
}

void Test_L1C_Way_Count()
{
    printf("**************************************************************\n");
    printf("L1 DCache Way Count Test\n");

    // L1_DCache大小
    register unsigned long cache_size = 1 << 15;
    // 申请 2 * cache_size 的数组空间
    BYTE* test_array = (BYTE*) malloc(cache_size << 1);
    // 组相联测试次数
    register unsigned long way_count_test_time = 1 << 20;
    // 将数组平均分成 2^n 组
    for (register unsigned long group_size = (1 << 2); group_size < (1 << 8); group_size <<= 1) {
        // 确定最大组数
        register unsigned long group_max_index = (cache_size << 1) / group_size;
        // 刷新L2Cache
        Clear_L2_Cache();
        // 记录耗时
        struct timeval tp[2];

        /*
         * 测试段
         */
        gettimeofday(&tp[0], NULL);
        // 保证访存次数相同
        int cnt = 0;
        while (cnt < way_count_test_time) {
            // 只访问奇数块
            for (register unsigned long group_index = 1; group_index < group_max_index; group_index += 2) {
                // 块内偏移量
                for (register unsigned long offset = 0; offset < group_size; offset++) {
                    test_array[group_index * group_size + offset] = cnt;
                    cnt++;
                }
            }
        }
        gettimeofday(&tp[1], NULL);

        // 输出结果
        printf("[Test_Split_Groups = %-3ld]\t Average access time : %.3f us\n", group_size, get_usec(tp[0], tp[1]));
    }
    free(test_array);
}

void Test_L2C_Way_Count()
{
    printf("**************************************************************\n");
    printf("L2 Cache Way Count Test\n");

    // TODO
}

void Test_Cache_Write_Policy()
{
    printf("**************************************************************\n");
    printf("Cache Write Policy Test\n");

    // TODO
}

void Test_Cache_Swap_Method()
{
    printf("**************************************************************\n");
    printf("Cache Replace Method Test\n");

    // TODO
}

void Test_TLB_Size()
{
    printf("**************************************************************\n");
    printf("TLB Size Test\n");

    const int page_size = 1 << 12;								// Execute "getconf PAGE_SIZE" under linux terminal

    // 测试次数
    register unsigned long TLB_test_time = 1 << 15;
    for (unsigned long group_size = (1 << 4); group_size < (1 << 9); group_size <<= 1) {
        // 申请测试数组
        BYTE* test_array = (BYTE*) malloc(ARRAY_SIZE);
        // 记录时间
        struct timeval tp[2];
        // 刷新L2Cache
        Clear_L2_Cache();

        /*
         * 测试段
         */
        gettimeofday(&tp[0], NULL);
        for (register unsigned long i = 0; i < TLB_test_time; i++) {
            register int index = (rand() % group_size) * page_size;
            test_array[index] = i;
        }
        gettimeofday(&tp[1], NULL);

        // 输出结果
        printf("[Test_TLB_entries = %-5ld]\t Average access time: %3f us\n", group_size, get_usec(tp[0], tp[1]));

        free(test_array);
    }
}

int main()
{
    Test_Cache_Size();
    Test_L1C_Block_Size();
    // Test_L2C_Block_Size();
    Test_L1C_Way_Count();
    // Test_L2C_Way_Count();
    // Test_Cache_Write_Policy();
    // Test_Cache_Swap_Method();
    Test_TLB_Size();

    return 0;
}
