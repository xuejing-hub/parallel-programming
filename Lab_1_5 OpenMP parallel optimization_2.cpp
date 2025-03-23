//基于OpenMP来完成思考更多算法设计思路来探索现代计算机体系结构中cache和超标量对程序性能的影响。
#include <iostream>
#include <omp.h>
#include <algorithm>
double parallel_sum(double* arr, int n) {
    double sum = 0.0;
#pragma omp parallel 
    {
        double local_sum = 0.0;
        int tid = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        int chunk_size = (n + num_threads - 1) / num_threads;
        int start = tid * chunk_size;
        int end = std::min(start + chunk_size, n);
        for (int i = start; i < end; i += 4) {
            local_sum += arr[i] + arr[i + 1] + arr[i + 2] + arr[i + 3];
        }
#pragma omp atomic
        sum += local_sum;
    }
    return sum;
}
