#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iomanip>
using namespace std;
using namespace std::chrono;
vector<vector<float>> Matrix(int n) {
    vector<vector<float>> Mat(n, vector<float>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            Mat[i][j] = (i + j) % 10;
        }
    }
    return Mat;
}
vector<float> Vector(int n) {
    vector<float> Vec(n);
    for (int i = 0; i < n; i++) {
        Vec[i] = i % 10;
    }
    return Vec;
}
// (1) 逐列访问元素的平凡算法
vector<float> Simple(const vector<vector<float>>& Mat, const vector<float>& Vec, int n) {
    vector<float> ResultVec(n, 0.0f);//浮点型！最好不要用0
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) {
            ResultVec[j] += Mat[i][j] * Vec[i];
        }
    }
    return ResultVec;
}
// (2) Cache 优化算法（逐行访问）
vector<float> Cache(const vector<vector<float>>& Mat, const vector<float>& Vec, int n) {
    vector<float> ResultVec(n, 0.0f);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            ResultVec[j] += Mat[i][j] * Vec[i];
        }
    }
    return ResultVec;
}
void Test(int n, int m = 5) {//5次，取平均，防止误差
    vector<vector<float>> Mat = Matrix(n);
    vector<float> Vec = Vector(n);
    double timesimple = 0, timecache = 0;
    for (int r = 0; r < m; r++) {
        auto start1 = high_resolution_clock::now();
        vector<float> result1 = Simple(Mat, Vec, n);
        auto stop1 = high_resolution_clock::now();
        auto duration1 = duration_cast<nanoseconds>(stop1 - start1).count() / 1e6;//纳秒转毫秒
        timesimple += duration1;
        auto start2 = high_resolution_clock::now();
        vector<float> result2 = Cache(Mat, Vec, n);
        auto stop2 = high_resolution_clock::now();
        auto duration2 = duration_cast<nanoseconds>(stop2 - start2).count() / 1e6;
        timecache += duration2;
    }
    cout << fixed << setprecision(5);
    cout << "逐列访问的平凡算法 平均时间: " << (timesimple / m) << " ms" << endl;
    cout << "Cache 优化算法 平均时间: " << (timecache / m) << " ms" << endl;
}
int main() {
    Test(256); Test(512); Test(1024); Test(2048);
    return 0;
}
