#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5.h"
#include <iomanip>
using namespace std;
using namespace chrono;

// 编译指令如下：
// g++ correctness.cpp train.cpp guessing.cpp md5.cpp -o test.exe


int main() {
    bit32 state[4];
    MD5Hash("xuejing", state);
    for (int i1 = 0; i1 < 4; i1 += 1)
    {
        cout << std::setw(8) << std::setfill('0') << hex << state[i1];
    }
    cout << endl;
    
    string inputs[4] = {
        "xuejing",
        "yongyuan",
        "xihuan",
        "liuyang"
    };
    for (int i = 0; i < 4; ++i) {
        bit32 state[4][4];
        SIMD_MD5Hash(inputs, state);
        cout << "MD5(" << inputs[i] << ") = ";
        for (int j = 0; j < 4; j++) {
            cout << std::setw(8) << std::setfill('0') << hex << state[i][j];
        }
        cout << endl;
    }
    return 0;
}
