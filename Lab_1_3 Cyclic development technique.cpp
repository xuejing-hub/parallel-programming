//可采用循环展开（unroll）技术，降低循环操作（循环判定、归纳变量递增/递减等）对性能的影响，甚至可以采用宏/模板技术彻底消除循环。
#include <iostream>
#include <vector>
#include <chrono>
using namespace std;
vector<float> GetArr(int n) {
    vector<float> arr(n);
    for (int i = 0; i < n; i++) {
        arr[i] = i % 10;
    }
    return arr;
}
// 逐个累加的平凡算法（链式）
float Simple(const vector<float>& arr, int n) {
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
    for (; i <= n - 4; i += 4) {
        sum1 += arr[i];
        sum2 += arr[i + 1];
        sum3 += arr[i + 2];
        sum4 += arr[i + 3];
    }
    for (; i < n; i++) {
        sum1 += arr[i];
    }
    return sum1 + sum2 + sum3 + sum4;
}
// 8路循环展开并行累加
float Parallel8(const vector<float>& arr, int n) {
    float sum1 = 0.0f, sum2 = 0.0f, sum3 = 0.0f, sum4 = 0.0f;
    float sum5 = 0.0f, sum6 = 0.0f, sum7 = 0.0f, sum8 = 0.0f;
    int i = 0;
    for (; i <= n - 8; i += 8) {
        sum1 += arr[i];
        sum2 += arr[i + 1];
        sum3 += arr[i + 2];
        sum4 += arr[i + 3];
        sum5 += arr[i + 4];
        sum6 += arr[i + 5];
        sum7 += arr[i + 6];
        sum8 += arr[i + 7];
    }
    for (; i < n; i++) {
        sum1 += arr[i];
    }
    return sum1 + sum2 + sum3 + sum4 + sum5 + sum6 + sum7 + sum8;
}
// 递归算法
float RecursionOptimized(const vector<float>& arr, int left, int right) {
    if (right - left < 100) {
        float sum = 0.0f;
        for (int i = left; i <= right; i++) {
            sum += arr[i];
        }
        return sum;
    }
    int mid = (left + right) / 2;
    float leftSum = RecursionOptimized(arr, left, mid);
    float rightSum = RecursionOptimized(arr, mid + 1, right);
    return leftSum + rightSum;
}
int main() {
    int n = 1000000;
    vector<float> arr = GetArr(n);
    auto start = chrono::high_resolution_clock::now();
    float result1 =Simple(arr, n);
    auto end = chrono::high_resolution_clock::now();
    cout << "逐个累加的平凡算法（链式）的时间: "
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    start = chrono::high_resolution_clock::now();
    float result2 = Parallel4(arr, n);
    end = chrono::high_resolution_clock::now();
    cout << "4路循环展开并行累加的时间: "
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    start = chrono::high_resolution_clock::now();
    float result3 = Parallel8(arr, n);
    end = chrono::high_resolution_clock::now();
    cout << "8路循环展开并行累加的时间: "
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    start = chrono::high_resolution_clock::now();
    float result4 = RecursionOptimized(arr, 0, n - 1);
    end = chrono::high_resolution_clock::now();
    cout << "递归算法的时间: "
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    return 0;
}
