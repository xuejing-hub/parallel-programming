//基于OpenMP来完成做更多实验和分析，如探讨编译器不同优化力度对性能的影响；浮点数运算次序的不同对结果的影响等等。
#include <iostream>
#include <omp.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>

// 生成随机数据
void generate_data(std::vector<double>& data) {
#pragma omp parallel for
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<double>(rand()) / RAND_MAX;
    }
}

// 方式 1：直接按顺序累加
double sequential_sum(const std::vector<double>& data) {
    double sum = 0.0;
    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i];
    }
    return sum;
}

// 方式 2：OpenMP 递归分块求和，改变计算顺序
double parallel_sum(const std::vector<double>& data) {
    double sum = 0.0;
#pragma omp parallel
    {
        double local_sum = 0.0;
#pragma omp for nowait
        for (size_t i = 0; i < data.size(); ++i) {
            local_sum += data[i];  // 可能导致浮点误差
        }
#pragma omp atomic
        sum += local_sum;
    }
    return sum;
}

// 方式 3：分块归约求和，保证相对稳定的计算顺序
double parallel_reduction_sum(const std::vector<double>& data) {
    double sum = 0.0;
#pragma omp parallel for reduction(+:sum)
    for (size_t i = 0; i < data.size(); ++i) {
        sum += data[i];  // 采用 reduction 避免并发写入误差
    }
    return sum;
}

int main() {
    const size_t N = 10000000;
    std::vector<double> data(N);

    srand(time(nullptr));
    generate_data(data);

    double t1, t2, result;

    // 顺序求和
    t1 = omp_get_wtime();
    result = sequential_sum(data);
    t2 = omp_get_wtime();
    std::cout << "Sequential sum: " << std::setprecision(15) << result
        << " | Time: " << t2 - t1 << " s\n";

    // OpenMP 直接并行求和（可能有浮点误差）
    t1 = omp_get_wtime();
    result = parallel_sum(data);
    t2 = omp_get_wtime();
    std::cout << "Parallel sum (atomic): " << std::setprecision(15) << result
        << " | Time: " << t2 - t1 << " s\n";

    // OpenMP 归约求和（稳定计算顺序）
    t1 = omp_get_wtime();
    result = parallel_reduction_sum(data);
    t2 = omp_get_wtime();
    std::cout << "Parallel sum (reduction): " << std::setprecision(15) << result
        << " | Time: " << t2 - t1 << " s\n";

    return 0;
}
