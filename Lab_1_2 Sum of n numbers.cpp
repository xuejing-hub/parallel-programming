//计算n个数的和，考虑两种算法设计思路：
//逐个累加的平凡算法（链式）。
//超标量优化算法（指令级并行），如最简单的两路链式累加；再如递归算法——两两相加、中间结果再两两相加，依次类推，直至只剩下最终结果。
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
float Result(const vector<float>& arr, int n) {
    float result = 0.0f;
    for (int i = 0; i < n; i++) {
        result += arr[i];
    }
    return result;
}
//两路链式累加
float Parallel(const vector<float>& arr, int n) {
    float sum1 = 0.0f, sum2 = 0.0f;
    for (int i = 0; i < n - 1; i += 2) {
        sum1 += arr[i];
        sum2 += arr[i + 1];
    }
    if (n % 2 == 1) {
        sum1 += arr[n - 1];
    }
    return sum1 + sum2;
}
//递归算法
float Recursion(const vector<float>& arr, int left, int right) {
    if (right - left < 32) { 
        float sum = 0.0f;
        for (int i = left; i <= right; i++) {
            sum += arr[i];
        }
        return sum;
    }
    int mid = left + (right - left) / 2;
    return Recursion(arr, left, mid) + Recursion(arr, mid + 1, right);
}
int main() {
    int n = 1000000;
    vector<float> arr = GetArr(n);
    auto start = chrono::high_resolution_clock::now();
    float result1 = Result(arr, n);
    auto end = chrono::high_resolution_clock::now();
    cout << "逐个累加的平凡算法（链式）的时间："
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    start = chrono::high_resolution_clock::now();
    float result2 = Parallel(arr, n);
    end = chrono::high_resolution_clock::now();
    cout << "两路链式累加的时间："
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    start = chrono::high_resolution_clock::now();
    float result3 = Recursion(arr, 0, n - 1);
    end = chrono::high_resolution_clock::now();
    cout << "递归算法的时间: "
        << chrono::duration<double, milli>(end - start).count() << " ms\n";
    return 0;
}
