//SIMD向量化
#include <iostream>
#include <vector>
#include <chrono>
#include <immintrin.h>  // 包含SIMD相关的头文件

using namespace std;
using namespace std::chrono;

// 生成测试数据
vector<float> GetArr(int n) {
    vector<float> arr(n);
    for (int i = 0; i < n; i++) {
        arr[i] = i % 10;  // 填充一些数据进行测试
    }
    return arr;
}

// SIMD 向量化优化的求和方法
float SIMD_Sum(const vector<float>& arr, int n) {
    __m128 sumVec = _mm_setzero_ps();  // 创建一个全为零的SIMD向量
    int i = 0;

    // 遍历数组，每次处理4个数据
    for (; i + 3 < n; i += 4) {
        __m128 vec = _mm_loadu_ps(&arr[i]);  // 加载4个浮点数到SIMD寄存器
        sumVec = _mm_add_ps(sumVec, vec);  // 累加到sumVec
    }

    // 将累加的SIMD寄存器内容提取到一个数组
    float sumArray[4];
    _mm_storeu_ps(sumArray, sumVec);

    // 将4个累加的值相加
    float sum = sumArray[0] + sumArray[1] + sumArray[2] + sumArray[3];

    // 处理数组剩余部分（不足4个元素）
    for (; i < n; i++) {
        sum += arr[i];
    }

    return sum;  // 返回最终结果
}

// 测试SIMD优化的执行时间
void TestSIMD(int n) {
    vector<float> arr = GetArr(n);

    auto start = high_resolution_clock::now();
    float result = SIMD_Sum(arr, n);
    auto end = high_resolution_clock::now();

    cout << "SIMD 向量化优化的时间: " << duration<double, milli>(end - start).count() << " ms\n";
    cout << "计算结果: " << result << endl;
}

int main() {
    cout << "测试数组大小: 1000000\n";
    TestSIMD(1000000);
    return 0;
}
