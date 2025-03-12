//鼓励开放思路，思考更多算法设计思路来探索现代计算机体系结构中cache和超标量对程序性能的影响。
#include <iostream>
#include <vector>
#include <chrono>
#include <omp.h>
using namespace std;
vector<float> GetArr(int n) {
    vector<float> arr(n);
    for (int i = 0; i < n; i++) {
        arr[i] = i % 10;
    }
    return arr;
}
// 逐个累加的平凡算法（链式）
float SerialSum(const vector<float>& arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}
// 4路循环展开并行累加
float Parallel4(const vector<float>& arr, int n) {
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
    int i = 0;
    for (; i + 3 < n; i += 4) {
        sum1 += arr[i];
        sum2 += arr[i + 1];
        sum3 += arr[i + 2];
        sum4 += arr[i + 3];
    }
    for (; i < n; i++) sum1 += arr[i];
    return sum1 + sum2 + sum3 + sum4;
}

// 8路循环展开并行累加
float Parallel8(const vector<float>& arr, int n) {
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
    float sum5 = 0.0f, sum6 = 0.0f, sum7 = 0.0f, sum8 = 0.0f;
    int i = 0;
    for (; i + 7 < n; i += 8) { 
        sum1 += arr[i];
        sum2 += arr[i + 1];
        sum3 += arr[i + 2];
        sum4 += arr[i + 3];
        sum5 += arr[i + 4];
        sum6 += arr[i + 5];
        sum7 += arr[i + 6];
        sum8 += arr[i + 7];
    }
    for (; i < n; i++) sum1 += arr[i];
    return sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8;
}
// OpenMP 并行优化
float ParallelOpenMP(const vector<float>& arr, int n) {
    float sum = 0.0f;
#pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}
int main() {
    int n = 10000000; 
    vector<float> arr = GetArr(n);
    auto start = chrono::high_resolution_clock::now();
    float result1 = SerialSum(arr, n);
    auto end = chrono::high_resolution_clock::now();
    cout << "[Serial Sum] Value: " << result1 << " | Time: "
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    start = chrono::high_resolution_clock::now();
    float result2 = Parallel4(arr, n);
    end = chrono::high_resolution_clock::now();
    cout << "[Unroll (4)] Value: " << result2 << " | Time: "
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    start = chrono::high_resolution_clock::now();
    float result3 = Parallel8(arr, n);
    end = chrono::high_resolution_clock::now();
    cout << "[Unroll (8)] Value: " << result3 << " | Time: "
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    start = chrono::high_resolution_clock::now();
    float result4 = ParallelOpenMP(arr, n);
    end = chrono::high_resolution_clock::now();
    cout << "[Parallel OpenMP] Value: " << result4 << " | Time: "
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    return 0;
}
