#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5.h"
#include <iomanip>
#include<iostream>
using namespace std;
using namespace chrono;

// 编译指令如下
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O1
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O2 -fopenmp -std=c++11


// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -fopenmp -std=c++11
// bash test.sh 3 1 4


// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -lpthread -std=c++11
// bash test.sh 2 1 4

//qdel .master_ubss1

int main()
{
    double time_hash = 0;  // 用于MD5哈希的时间
    double time_guess = 0; // 哈希和猜测的总时长
    double time_train = 0; // 模型训练的总时长
    PriorityQueue q;
    auto start_train = system_clock::now();
    q.m.train("/guessdata/Rockyou-singleLined-full.txt");
    q.m.order();
    auto end_train = system_clock::now();
    auto duration_train = duration_cast<microseconds>(end_train - start_train);
    time_train = double(duration_train.count()) * microseconds::period::num / microseconds::period::den;

    q.init();
    cout << "here" << endl;
    int curr_num = 0;
    auto start = system_clock::now();
    // 由于需要定期清空内存，我们在这里记录已生成的猜测总数
    int history = 0;
    // std::ofstream a("./files/results.txt");
    while (!q.priority.empty())
    {
        q.PopNext();
        q.total_guesses = q.guesses.size();
        if (q.total_guesses - curr_num >= 100000)
        {
            cout << "Guesses generated: " <<history + q.total_guesses << endl;
            curr_num = q.total_guesses;

            // 在此处更改实验生成的猜测上限
            int generate_n=10000000;
            if (history + q.total_guesses > 10000000)
            {
                auto end = system_clock::now();
                auto duration = duration_cast<microseconds>(end - start);
                time_guess = double(duration.count()) * microseconds::period::num / microseconds::period::den;
                cout << "Guess time:" << time_guess - time_hash << "seconds"<< endl;
                cout << "Hash time:" << time_hash << "seconds"<<endl;
                cout << "Train time:" << time_train <<"seconds"<<endl;
                break;
            }
        }
        // 为了避免内存超限，我们在q.guesses中口令达到一定数目时，将其中的所有口令取出并且进行哈希
        // 然后，q.guesses将会被清空。为了有效记录已经生成的口令总数，维护一个history变量来进行记录
        // 在main.cpp中修改
        if (curr_num > 1000000) {
            auto start_hash = system_clock::now();
            bit32 state[4][4];
            size_t n = q.guesses.size();
            
            // #pragma omp parallel for schedule(static)
            for (size_t i = 0; i < n; i += 4) {
                std::string inputs[4] = { "", "", "", "" };
                int batch = std::min<size_t>(4, n - i);
                for (int j = 0; j < batch; ++j) {
                    inputs[j] = std::move(q.guesses[i + j]);
                }
                SIMD_MD5Hash(inputs, state);
            }


            // bit32 state[4];
            // for (string pw : q.guesses)
            // {
            //     MD5Hash(pw, state);
            // }
            auto end_hash = system_clock::now();
            auto duration = duration_cast<microseconds>(end_hash - start_hash);
            time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;
            
            history += curr_num;
            curr_num = 0;
            q.guesses.clear();
        }
    }
}
