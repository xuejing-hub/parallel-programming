//MPI之前的版本
#include "PCFG.h"
#include <chrono>
#include <fstream>
#include "md5.h"
#include <iomanip>
#include <omp.h>
#include <unordered_set>
using namespace std;
using namespace chrono;
// mpic++ correctness_guess.cpp train.cpp guessing.cpp md5.cpp -o main -fopenmp -std=c++11
// bash test.sh 3 1 4

// g++ correctness_guess.cpp train.cpp guessing.cpp md5.cpp -o main -lpthread -std=c++11
// bash test.sh 2 1 4

// 编译指令如下
// mpic++ main.cpp train.cpp guessing.cpp md5.cpp -o main
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O1
// g++ main.cpp train.cpp guessing.cpp md5.cpp -o main -O2

//nvcc correctness_guess.cpp train.cpp guessing.cpp guessing_cuda.cu md5.cpp -o main -std=c++17 -Xcompiler -fopenmp
//./main
//qsub qsub_mpi.sh
//qdel .master_ubss1
//qstat -u s2312175

int main()
{
    double time_hash = 0;  // 用于MD5哈希的时间
    double time_guess = 0; // 哈希和猜测的总时长
    double time_train = 0; // 模型训练的总时长
    PriorityQueue q;
    auto start_train = system_clock::now();
    // q.m.train("/guessdata/Rockyou-singleLined-full.txt");
    q.m.train("Rockyou-singleLined-full.txt");
    q.m.order();
    auto end_train = system_clock::now();
    auto duration_train = duration_cast<microseconds>(end_train - start_train);
    time_train = double(duration_train.count()) * microseconds::period::num / microseconds::period::den;

    // 加载一些测试数据
    unordered_set<std::string> test_set;
    // ifstream test_data("/guessdata/Rockyou-singleLined-full.txt");
    ifstream test_data("Rockyou-singleLined-full.txt");
    int test_count = 0;
    string pw;
    while (test_data >> pw)
    {
        test_count += 1;
        test_set.insert(pw);
        if (test_count >= 1000000)
        {
            break;
        }
    }
    int cracked = 0;

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
            cout << "Guesses generated: " << history + q.total_guesses << endl;
            curr_num = q.total_guesses;

            // 在此处更改实验生成的猜测上限
            int generate_n = 10000000;
            if (history + q.total_guesses > generate_n)
            {
                auto end = system_clock::now();
                auto duration = duration_cast<microseconds>(end - start);
                time_guess = double(duration.count()) * microseconds::period::num / microseconds::period::den;
                cout << "Guess time:" << time_guess - time_hash << "seconds" << endl;
                cout << "Hash time:" << time_hash << "seconds" << endl;
                cout << "Train time:" << time_train << "seconds" << endl;
                cout << "Cracked:" << cracked << endl;
                break;
            }
        }
        // 为了避免内存超限，我们在q.guesses中口令达到一定数目时，将其中的所有口令取出并且进行哈希
        // 然后，q.guesses将会被清空。为了有效记录已经生成的口令总数，维护一个history变量来进行记录
        if (curr_num > 1000000)
        {   
            auto start_hash = system_clock::now();
            bit32 state[4];
            for (string pw : q.guesses)
            {
                if (test_set.find(pw) != test_set.end()) {
                    cracked+=1;
                }
                MD5Hash(pw, state);
            }
            
            // 在这里对哈希所需的总时长进行计算
            auto end_hash = system_clock::now();
            auto duration = duration_cast<microseconds>(end_hash - start_hash);
            time_hash += double(duration.count()) * microseconds::period::num / microseconds::period::den;

            // 记录已经生成的口令总数
            history += curr_num;
            curr_num = 0;
            q.guesses.clear();
        }
    }
}




//MPI之后的版本
//mpic++ correctness_guess.cpp train.cpp guessing.cpp md5.cpp -o main
//qsub qsub_mpi.sh
//qdel .master_ubss1
//qstat -u s2312175

// #include <mpi.h>
// #include "PCFG.h"
// #include <chrono>
// #include <fstream>
// #include "md5.h"
// #include <iomanip>
// #include <unordered_set>
// using namespace std;

// int main(int argc, char* argv[])
// {
//     MPI_Init(&argc, &argv);

//     double time_hash = 0;  // 用于MD5哈希的时间
//     double time_guess = 0; // 哈希和猜测的总时长
//     double time_train = 0; // 模型训练的总时长
//     PriorityQueue q;

//     double start_train = MPI_Wtime();
//     q.m.train("/guessdata/Rockyou-singleLined-full.txt");
//     q.m.order();
//     double end_train = MPI_Wtime();
//     time_train = end_train - start_train;

//     // 加载一些测试数据
//     unordered_set<std::string> test_set;
//     ifstream test_data("/guessdata/Rockyou-singleLined-full.txt");
//     int test_count = 0;
//     string pw;
//     while (test_data >> pw)
//     {
//         test_count += 1;
//         test_set.insert(pw);
//         if (test_count >= 1000000)
//         {
//             break;
//         }
//     }
//     int cracked = 0;

//     q.init();
//     cout << "here" << endl;
//     int curr_num = 0;
//     double start = MPI_Wtime();
//     int history = 0;

//     while (!q.priority.empty())
//     {
//         q.PopNext();
//         q.total_guesses = q.guesses.size();
//         if (q.total_guesses - curr_num >= 100000)
//         {
//             cout << "Guesses generated: " << history + q.total_guesses << endl;
//             curr_num = q.total_guesses;

//             int generate_n = 10000000;
//             if (history + q.total_guesses > generate_n)
//             {
//                 double end = MPI_Wtime();
//                 time_guess = end - start;
//                 cout << "Guess time:" << time_guess - time_hash << "seconds" << endl;
//                 cout << "Hash time:" << time_hash << "seconds" << endl;
//                 cout << "Train time:" << time_train << "seconds" << endl;
//                 cout << "Cracked:" << cracked << endl;
//                 break;
//             }
//         }

//         if (curr_num > 1000000)
//         {
//             double start_hash = MPI_Wtime();
//             bit32 state[4];
//             for (string pw : q.guesses)
//             {
//                 if (test_set.find(pw) != test_set.end()) {
//                     cracked += 1;
//                 }
//                 MD5Hash(pw, state);
//             }

//             double end_hash = MPI_Wtime();
//             time_hash += (end_hash - start_hash);

//             history += curr_num;
//             curr_num = 0;
//             q.guesses.clear();
//         }
//     }

//     MPI_Finalize();
//     return 0;
// }










// #include <thread>
// #include <queue>
// #include <mutex>
// #include <condition_variable>
// #include <vector>
// #include <atomic>
// #include <unordered_set>
// #include <fstream>
// #include <iostream>
// #include <iomanip>
// #include <chrono>
// #include "PCFG.h"
// #include "md5.h"
// using namespace std;

// queue<vector<string>> guess_queue;
// mutex mtx;
// condition_variable cv;
// atomic<bool> done(false);

// void hash_worker(const unordered_set<string>& test_set, atomic<int>& cracked, double& time_hash) {
//     while (true) {
//         vector<string> batch;
//         {
//             unique_lock<mutex> lock(mtx);
//             cv.wait(lock, []{ return !guess_queue.empty() || done; });
//             if (guess_queue.empty() && done) break;
//             batch = std::move(guess_queue.front());
//             guess_queue.pop();
//         }
//         auto start_hash = chrono::system_clock::now();
//         bit32 state[4];
//         for (auto& pw : batch) {
//             if (test_set.find(pw) != test_set.end()) cracked++;
//             MD5Hash(pw, state);
//         }
//         auto end_hash = chrono::system_clock::now();
//         auto duration = chrono::duration_cast<chrono::microseconds>(end_hash - start_hash);
//         time_hash += double(duration.count()) * chrono::microseconds::period::num / chrono::microseconds::period::den;
//     }
// }

// int main(int argc, char* argv[])
// {
//     // 训练模型
//     double time_hash = 0;
//     double time_guess = 0;
//     double time_train = 0;
//     PriorityQueue q;

//     auto start_train = chrono::system_clock::now();
//     q.m.train("/guessdata/Rockyou-singleLined-full.txt");
//     q.m.order();
//     auto end_train = chrono::system_clock::now();
//     auto duration_train = chrono::duration_cast<chrono::microseconds>(end_train - start_train);
//     time_train = double(duration_train.count()) * chrono::microseconds::period::num / chrono::microseconds::period::den;

//     // 加载测试数据
//     unordered_set<string> test_set;
//     ifstream test_data("/guessdata/Rockyou-singleLined-full.txt");
//     int test_count = 0;
//     string pw;
//     while (test_data >> pw) {
//         test_count += 1;
//         test_set.insert(pw);
//         if (test_count >= 1000000) break;
//     }
//     atomic<int> cracked(0);

//     q.init();
//     cout << "here" << endl;
//     int curr_num = 0;
//     int history = 0;
//     auto start = chrono::system_clock::now();

//     // 启动哈希线程
//     thread hash_thread(hash_worker, cref(test_set), ref(cracked), ref(time_hash));

//     const int BATCH_SIZE = 1000000;
//     while (!q.priority.empty()) {
//         q.PopNext();
//         q.total_guesses = q.guesses.size();
//         if (q.total_guesses - curr_num >= 100000) {
//             cout << "Guesses generated: " << history + q.total_guesses << endl;
//             curr_num = q.total_guesses;
//             int generate_n = 10000000;
//             if (history + q.total_guesses > generate_n) {
//                 auto end = chrono::system_clock::now();
//                 auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
//                 time_guess = double(duration.count()) * chrono::microseconds::period::num / chrono::microseconds::period::den;
//                 // 通知哈希线程退出
//                 {
//                     unique_lock<mutex> lock(mtx);
//                     done = true;
//                 }
//                 cv.notify_one();
//                 hash_thread.join();
//                 cout << "Guess time:" << time_guess<< "seconds" << endl;
//                 cout << "Hash time:" << time_hash << "seconds" << endl;
//                 cout << "Train time:" << time_train << "seconds" << endl;
//                 cout << "Cracked:" << cracked << endl;
//                 return 0;
//             }
//         }
//         // 达到批量阈值，送去哈希线程
//         if (curr_num > BATCH_SIZE) {
//             vector<string> batch;
//             batch.swap(q.guesses);
//             {
//                 unique_lock<mutex> lock(mtx);
//                 guess_queue.push(std::move(batch));
//             }
//             cv.notify_one();
//             history += curr_num;
//             curr_num = 0;
//         }
//     }
//     // 处理剩余未哈希的口令
//     if (!q.guesses.empty()) {
//         vector<string> batch;
//         batch.swap(q.guesses);
//         {
//             unique_lock<mutex> lock(mtx);
//             guess_queue.push(std::move(batch));
//         }
//         cv.notify_one();
//     }
//     // 通知哈希线程退出
//     {
//         unique_lock<mutex> lock(mtx);
//         done = true;
//     }
//     cv.notify_one();
//     hash_thread.join();

//     auto end = chrono::system_clock::now();
//     auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
//     time_guess = double(duration.count()) * chrono::microseconds::period::num / chrono::microseconds::period::den;
//     cout << "Guess time:" << time_guess<< "seconds" << endl;
//     cout << "Hash time:" << time_hash << "seconds" << endl;
//     cout << "Train time:" << time_train << "seconds" << endl;
//     cout << "Cracked:" << cracked << endl;
//     return 0;
// }