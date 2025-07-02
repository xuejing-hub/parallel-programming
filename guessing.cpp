#include "PCFG.h"
#include <pthread.h>
#include <unistd.h>
#include <mutex>
#include <vector>
#include <string>
#include <omp.h>
#include <sstream>
#include <cstring>
#include <thread>
#include <functional>      
#include <algorithm>
#include <atomic>
//#include <mpi.h>
#include <numeric>
#include <cuda_runtime.h>

using namespace std;

#define MAX_STR_LEN 64

extern "C" {
void launch_generate_single_segment(char **values, char *output, int count, int blockSize, int gridSize);
void launch_generate_multi_segment(char *prefix, char **suffixes, char *output, int count, int blockSize, int gridSize);
}

void PriorityQueue::CalProb(PT &pt)
{
    // 计算PriorityQueue里面一个PT的流程如下：
    // 1. 首先需要计算一个PT本身的概率。例如，L6S1的概率为0.15
    // 2. 需要注意的是，Queue里面的PT不是“纯粹的”PT，而是除了最后一个segment以外，全部被value实例化的PT
    // 3. 所以，对于L6S1而言，其在Queue里面的实际PT可能是123456S1，其中“123456”为L6的一个具体value。
    // 4. 这个时候就需要计算123456在L6中出现的概率了。假设123456在所有L6 segment中的概率为0.1，那么123456S1的概率就是0.1*0.15

    // 计算一个PT本身的概率。后续所有具体segment value的概率，直接累乘在这个初始概率值上
    pt.prob = pt.preterm_prob;

    // index: 标注当前segment在PT中的位置
    int index = 0;


    for (int idx : pt.curr_indices)
    {
        // pt.content[index].PrintSeg();
        if (pt.content[index].type == 1)
        {
            // 下面这行代码的意义：
            // pt.content[index]：目前需要计算概率的segment
            // m.FindLetter(seg): 找到一个letter segment在模型中的对应下标
            // m.letters[m.FindLetter(seg)]：一个letter segment在模型中对应的所有统计数据
            // m.letters[m.FindLetter(seg)].ordered_values：一个letter segment在模型中，所有value的总数目
            pt.prob *= m.letters[m.FindLetter(pt.content[index])].ordered_freqs[idx];
            pt.prob /= m.letters[m.FindLetter(pt.content[index])].total_freq;
            // cout << m.letters[m.FindLetter(pt.content[index])].ordered_freqs[idx] << endl;
            // cout << m.letters[m.FindLetter(pt.content[index])].total_freq << endl;
        }
        if (pt.content[index].type == 2)
        {
            pt.prob *= m.digits[m.FindDigit(pt.content[index])].ordered_freqs[idx];
            pt.prob /= m.digits[m.FindDigit(pt.content[index])].total_freq;
            // cout << m.digits[m.FindDigit(pt.content[index])].ordered_freqs[idx] << endl;
            // cout << m.digits[m.FindDigit(pt.content[index])].total_freq << endl;
        }
        if (pt.content[index].type == 3)
        {
            pt.prob *= m.symbols[m.FindSymbol(pt.content[index])].ordered_freqs[idx];
            pt.prob /= m.symbols[m.FindSymbol(pt.content[index])].total_freq;
            // cout << m.symbols[m.FindSymbol(pt.content[index])].ordered_freqs[idx] << endl;
            // cout << m.symbols[m.FindSymbol(pt.content[index])].total_freq << endl;
        }
        index += 1;
    }
    // cout << pt.prob << endl;
}

void PriorityQueue::init()
{
    // cout << m.ordered_pts.size() << endl;
    // 用所有可能的PT，按概率降序填满整个优先队列
    for (PT pt : m.ordered_pts)
    {
        for (segment seg : pt.content)
        {
            if (seg.type == 1)
            {
                // 下面这行代码的意义：
                // max_indices用来表示PT中各个segment的可能数目。例如，L6S1中，假设模型统计到了100个L6，那么L6对应的最大下标就是99
                // （但由于后面采用了"<"的比较关系，所以其实max_indices[0]=100）
                // m.FindLetter(seg): 找到一个letter segment在模型中的对应下标
                // m.letters[m.FindLetter(seg)]：一个letter segment在模型中对应的所有统计数据
                // m.letters[m.FindLetter(seg)].ordered_values：一个letter segment在模型中，所有value的总数目
                pt.max_indices.emplace_back(m.letters[m.FindLetter(seg)].ordered_values.size());
            }
            if (seg.type == 2)
            {
                pt.max_indices.emplace_back(m.digits[m.FindDigit(seg)].ordered_values.size());
            }
            if (seg.type == 3)
            {
                pt.max_indices.emplace_back(m.symbols[m.FindSymbol(seg)].ordered_values.size());
            }
        }
        pt.preterm_prob = float(m.preterm_freq[m.FindPT(pt)]) / m.total_preterm;
        // pt.PrintPT();
        // cout << " " << m.preterm_freq[m.FindPT(pt)] << " " << m.total_preterm << " " << pt.preterm_prob << endl;

        // 计算当前pt的概率
        CalProb(pt);
        // 将PT放入优先队列
        priority.emplace_back(pt);
    }
    // cout << "priority size:" << priority.size() << endl;
}

void PriorityQueue::PopNext()
{

    // 对优先队列最前面的PT，首先利用这个PT生成一系列猜测
    Generate(priority.front());

    // 然后需要根据即将出队的PT，生成一系列新的PT
    vector<PT> new_pts = priority.front().NewPTs();
    for (PT pt : new_pts)
    {
        // 计算概率
        CalProb(pt);
        // 接下来的这个循环，作用是根据概率，将新的PT插入到优先队列中
        for (auto iter = priority.begin(); iter != priority.end(); iter++)
        {
            // 对于非队首和队尾的特殊情况
            if (iter != priority.end() - 1 && iter != priority.begin())
            {
                // 判定概率
                if (pt.prob <= iter->prob && pt.prob > (iter + 1)->prob)
                {
                    priority.emplace(iter + 1, pt);
                    break;
                }
            }
            if (iter == priority.end() - 1)
            {
                priority.emplace_back(pt);
                break;
            }
            if (iter == priority.begin() && iter->prob < pt.prob)
            {
                priority.emplace(iter, pt);
                break;
            }
        }
    }

    // 现在队首的PT善后工作已经结束，将其出队（删除）
    priority.erase(priority.begin());
}

// 这个函数你就算看不懂，对并行算法的实现影响也不大
// 当然如果你想做一个基于多优先队列的并行算法，可能得稍微看一看了
vector<PT> PT::NewPTs()
{
    // 存储生成的新PT
    vector<PT> res;

    // 假如这个PT只有一个segment
    // 那么这个segment的所有value在出队前就已经被遍历完毕，并作为猜测输出
    // 因此，所有这个PT可能对应的口令猜测已经遍历完成，无需生成新的PT
    if (content.size() == 1)
    {
        return res;
    }
    else
    {
        // 最初的pivot值。我们将更改位置下标大于等于这个pivot值的segment的值（最后一个segment除外），并且一次只更改一个segment
        // 上面这句话里是不是有没看懂的地方？接着往下看你应该会更明白
        int init_pivot = pivot;

        // 开始遍历所有位置值大于等于init_pivot值的segment
        // 注意i < curr_indices.size() - 1，也就是除去了最后一个segment（这个segment的赋值预留给并行环节）
        for (int i = pivot; i < curr_indices.size() - 1; i += 1)
        {
            // curr_indices: 标记各segment目前的value在模型里对应的下标
            curr_indices[i] += 1;

            // max_indices：标记各segment在模型中一共有多少个value
            if (curr_indices[i] < max_indices[i])
            {
                // 更新pivot值
                pivot = i;
                res.emplace_back(*this);
            }

            // 这个步骤对于你理解pivot的作用、新PT生成的过程而言，至关重要
            curr_indices[i] -= 1;
        }
        pivot = init_pivot;
        return res;
    }

    return res;
}

// gpu+cpu分情况
// extern "C" void launch_generate_single_segment_async(char **values, char *output, int count, int blockSize, int gridSize, cudaStream_t stream);
// extern "C" void launch_generate_multi_segment_async(char *prefix, char **suffixes, char *output, int count, int blockSize, int gridSize, cudaStream_t stream);

// void PriorityQueue::Generate(PT pt)
// {
//     CalProb(pt);

//     // 估计生成密码数（示例取第一个段的数量或最后一个段）
//     int n = 0;
//     if (!pt.max_indices.empty())
//         n = pt.max_indices.back(); // 你也可以根据实际改为其他估计方式

//     const int GPU_THRESHOLD = 1024; // 阈值：小于此值用CPU，大于用GPU（可调）

//     if (n < GPU_THRESHOLD)
//     {
//         // --- 用CPU计算 ---
//         if (pt.content.size() == 1)
//         {
//             // 单段，用OpenMP并行生成
//             segment* a = nullptr;
//             if (pt.content[0].type == 1)
//                 a = &m.letters[m.FindLetter(pt.content[0])];
//             else if (pt.content[0].type == 2)
//                 a = &m.digits[m.FindDigit(pt.content[0])];
//             else if (pt.content[0].type == 3)
//                 a = &m.symbols[m.FindSymbol(pt.content[0])];

//             int count = pt.max_indices[0];
//             #pragma omp parallel for
//             for (int i = 0; i < count; ++i)
//             {
//                 std::string s = a->ordered_values[i];
//                 if (!s.empty())
//                 {
//                     #pragma omp critical
//                     {
//                         guesses.emplace_back(s);
//                         total_guesses += 1;
//                     }
//                 }
//             }
//         }
//         else
//         {
//             // 多段串行生成，和你之前的CPU逻辑类似
//             // 先拼接前面段字符串
//             std::string prefix;
//             int seg_idx = 0;
//             for (int idx : pt.curr_indices)
//             {
//                 if (seg_idx >= (int)pt.content.size() - 1)
//                     break;

//                 if (pt.content[seg_idx].type == 1)
//                     prefix += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
//                 else if (pt.content[seg_idx].type == 2)
//                     prefix += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
//                 else if (pt.content[seg_idx].type == 3)
//                     prefix += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];

//                 seg_idx++;
//             }

//             // 最后一段逐个拼接后输出
//             segment* a = nullptr;
//             if (pt.content.back().type == 1)
//                 a = &m.letters[m.FindLetter(pt.content.back())];
//             else if (pt.content.back().type == 2)
//                 a = &m.digits[m.FindDigit(pt.content.back())];
//             else if (pt.content.back().type == 3)
//                 a = &m.symbols[m.FindSymbol(pt.content.back())];

//             int count = pt.max_indices.back();
//             #pragma omp parallel for
//             for (int i = 0; i < count; ++i)
//             {
//                 std::string s = prefix + a->ordered_values[i];
//                 if (!s.empty())
//                 {
//                     #pragma omp critical
//                     {
//                         guesses.emplace_back(s);
//                         total_guesses += 1;
//                     }
//                 }
//             }
//         }
//     }
//     else
//     {
//         // --- 用GPU异步计算 ---

//         cudaStream_t stream;
//         cudaStreamCreate(&stream);

//         if (pt.content.size() == 1)
//         {
//             segment* a = nullptr;
//             if (pt.content[0].type == 1)
//                 a = &m.letters[m.FindLetter(pt.content[0])];
//             else if (pt.content[0].type == 2)
//                 a = &m.digits[m.FindDigit(pt.content[0])];
//             else if (pt.content[0].type == 3)
//                 a = &m.symbols[m.FindSymbol(pt.content[0])];

//             int count = pt.max_indices[0];
//             std::vector<std::string> host_strs(count);

//             #pragma omp parallel for
//             for (int i = 0; i < count; ++i)
//                 host_strs[i] = a->ordered_values[i];

//             char* d_char_buf;
//             cudaMalloc(&d_char_buf, count * MAX_STR_LEN * sizeof(char));
//             char* h_char_buf = new char[count * MAX_STR_LEN];

//             #pragma omp parallel for
//             for (int i = 0; i < count; ++i)
//                 strncpy(h_char_buf + i * MAX_STR_LEN, host_strs[i].c_str(), MAX_STR_LEN);

//             cudaMemcpyAsync(d_char_buf, h_char_buf, count * MAX_STR_LEN * sizeof(char), cudaMemcpyHostToDevice, stream);

//             char** h_ptr_array = new char*[count];
//             for (int i = 0; i < count; ++i)
//                 h_ptr_array[i] = d_char_buf + i * MAX_STR_LEN;

//             char** d_ptr_array;
//             cudaMalloc(&d_ptr_array, count * sizeof(char*));
//             cudaMemcpyAsync(d_ptr_array, h_ptr_array, count * sizeof(char*), cudaMemcpyHostToDevice, stream);

//             char* d_output;
//             cudaMalloc(&d_output, count * MAX_STR_LEN * sizeof(char));

//             int blockSize = 256;
//             int gridSize = (count + blockSize - 1) / blockSize;

//             launch_generate_single_segment_async(d_ptr_array, d_output, count, blockSize, gridSize, stream);

//             cudaStreamSynchronize(stream);

//             std::vector<char> h_output(count * MAX_STR_LEN);
//             cudaMemcpy(h_output.data(), d_output, count * MAX_STR_LEN * sizeof(char), cudaMemcpyDeviceToHost);

//             #pragma omp parallel for
//             for (int i = 0; i < count; ++i)
//             {
//                 std::string s(h_output.data() + i * MAX_STR_LEN);
//                 if (!s.empty())
//                 {
//                     #pragma omp critical
//                     {
//                         guesses.emplace_back(s);
//                         total_guesses += 1;
//                     }
//                 }
//             }

//             delete[] h_char_buf;
//             delete[] h_ptr_array;
//             cudaFree(d_char_buf);
//             cudaFree(d_ptr_array);
//             cudaFree(d_output);
//         }
//         else
//         {
//             // 多段情况，先拼接 prefix
//             std::string prefix;
//             int seg_idx = 0;
//             for (int idx : pt.curr_indices)
//             {
//                 if (seg_idx >= (int)pt.content.size() - 1)
//                     break;

//                 if (pt.content[seg_idx].type == 1)
//                     prefix += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
//                 else if (pt.content[seg_idx].type == 2)
//                     prefix += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
//                 else if (pt.content[seg_idx].type == 3)
//                     prefix += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];

//                 seg_idx++;
//             }

//             segment* a = nullptr;
//             if (pt.content.back().type == 1)
//                 a = &m.letters[m.FindLetter(pt.content.back())];
//             else if (pt.content.back().type == 2)
//                 a = &m.digits[m.FindDigit(pt.content.back())];
//             else if (pt.content.back().type == 3)
//                 a = &m.symbols[m.FindSymbol(pt.content.back())];

//             int count = pt.max_indices.back();
//             std::vector<std::string> host_strs(count);

//             #pragma omp parallel for
//             for (int i = 0; i < count; ++i)
//                 host_strs[i] = a->ordered_values[i];

//             char* d_suffix_buf;
//             cudaMalloc(&d_suffix_buf, count * MAX_STR_LEN * sizeof(char));
//             char* h_suffix_buf = new char[count * MAX_STR_LEN];

//             #pragma omp parallel for
//             for (int i = 0; i < count; ++i)
//                 strncpy(h_suffix_buf + i * MAX_STR_LEN, host_strs[i].c_str(), MAX_STR_LEN);

//             cudaMemcpyAsync(d_suffix_buf, h_suffix_buf, count * MAX_STR_LEN * sizeof(char), cudaMemcpyHostToDevice, stream);

//             char** h_suffix_ptrs = new char*[count];
//             for (int i = 0; i < count; ++i)
//                 h_suffix_ptrs[i] = d_suffix_buf + i * MAX_STR_LEN;

//             char** d_suffix_ptrs;
//             cudaMalloc(&d_suffix_ptrs, count * sizeof(char*));
//             cudaMemcpyAsync(d_suffix_ptrs, h_suffix_ptrs, count * sizeof(char*), cudaMemcpyHostToDevice, stream);

//             char* d_prefix;
//             cudaMalloc(&d_prefix, MAX_STR_LEN * sizeof(char));
//             cudaMemcpyAsync(d_prefix, prefix.c_str(), MAX_STR_LEN, cudaMemcpyHostToDevice, stream);

//             char* d_output;
//             cudaMalloc(&d_output, count * MAX_STR_LEN * sizeof(char));

//             int blockSize = 256;
//             int gridSize = (count + blockSize - 1) / blockSize;

//             launch_generate_multi_segment_async(d_prefix, d_suffix_ptrs, d_output, count, blockSize, gridSize, stream);

//             cudaStreamSynchronize(stream);

//             std::vector<char> h_output(count * MAX_STR_LEN);
//             cudaMemcpy(h_output.data(), d_output, count * MAX_STR_LEN * sizeof(char), cudaMemcpyDeviceToHost);

//             #pragma omp parallel for
//             for (int i = 0; i < count; ++i)
//             {
//                 std::string s(h_output.data() + i * MAX_STR_LEN);
//                 if (!s.empty())
//                 {
//                     #pragma omp critical
//                     {
//                         guesses.emplace_back(s);
//                         total_guesses += 1;
//                     }
//                 }
//             }

//             delete[] h_suffix_buf;
//             delete[] h_suffix_ptrs;
//             cudaFree(d_suffix_buf);
//             cudaFree(d_suffix_ptrs);
//             cudaFree(d_prefix);
//             cudaFree(d_output);
//         }

//         cudaStreamDestroy(stream);
//     }
// }











// //gpu+openmp+异步
// extern "C" void launch_generate_single_segment_async(char **values, char *output, int count, int blockSize, int gridSize, cudaStream_t stream);
// extern "C" void launch_generate_multi_segment_async(char *prefix, char **suffixes, char *output, int count, int blockSize, int gridSize, cudaStream_t stream);

// void PriorityQueue::Generate(PT pt)
// {
//     CalProb(pt);

//     cudaStream_t stream;
//     cudaStreamCreate(&stream);

//     if (pt.content.size() == 1)
//     {
//         segment *a = nullptr;
//         if (pt.content[0].type == 1)
//             a = &m.letters[m.FindLetter(pt.content[0])];
//         if (pt.content[0].type == 2)
//             a = &m.digits[m.FindDigit(pt.content[0])];
//         if (pt.content[0].type == 3)
//             a = &m.symbols[m.FindSymbol(pt.content[0])];

//         int n = pt.max_indices[0];
//         std::vector<std::string> host_strs(n);

//         #pragma omp parallel for
//         for (int i = 0; i < n; ++i)
//             host_strs[i] = a->ordered_values[i];

//         char* d_char_buf;
//         cudaMalloc(&d_char_buf, n * MAX_STR_LEN * sizeof(char));
//         char* h_char_buf = new char[n * MAX_STR_LEN];

//         #pragma omp parallel for
//         for (int i = 0; i < n; ++i) {
//             strncpy(h_char_buf + i * MAX_STR_LEN, host_strs[i].c_str(), MAX_STR_LEN);
//         }

//         cudaMemcpyAsync(d_char_buf, h_char_buf, n * MAX_STR_LEN * sizeof(char), cudaMemcpyHostToDevice, stream);

//         char** h_ptr_array = new char*[n];
//         for (int i = 0; i < n; ++i) {
//             h_ptr_array[i] = d_char_buf + i * MAX_STR_LEN;
//         }

//         char** d_ptr_array;
//         cudaMalloc(&d_ptr_array, n * sizeof(char*));
//         cudaMemcpyAsync(d_ptr_array, h_ptr_array, n * sizeof(char*), cudaMemcpyHostToDevice, stream);

//         char* d_output;
//         cudaMalloc(&d_output, n * MAX_STR_LEN * sizeof(char));

//         int blockSize = 256;
//         int gridSize = (n + blockSize - 1) / blockSize;

//         // 异步调用核函数启动器
//         launch_generate_single_segment_async(d_ptr_array, d_output, n, blockSize, gridSize, stream);

//         // 等待GPU完成
//         cudaStreamSynchronize(stream);

//         std::vector<char> h_output(n * MAX_STR_LEN);
//         cudaMemcpy(h_output.data(), d_output, n * MAX_STR_LEN * sizeof(char), cudaMemcpyDeviceToHost);

//         #pragma omp parallel for
//         for (int i = 0; i < n; ++i) {
//             std::string s(h_output.data() + i * MAX_STR_LEN);
//             if (!s.empty()) {
//                 #pragma omp critical
//                 {
//                     guesses.emplace_back(s);
//                     total_guesses += 1;
//                 }
//             }
//         }

//         delete[] h_char_buf;
//         delete[] h_ptr_array;
//         cudaFree(d_char_buf);
//         cudaFree(d_ptr_array);
//         cudaFree(d_output);
//     }
//     else
//     {
//         std::string guess;
//         int seg_idx = 0;
//         for (int idx : pt.curr_indices)
//         {
//             if (pt.content[seg_idx].type == 1)
//                 guess += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
//             if (pt.content[seg_idx].type == 2)
//                 guess += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
//             if (pt.content[seg_idx].type == 3)
//                 guess += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];
//             seg_idx += 1;
//             if (seg_idx == pt.content.size() - 1)
//                 break;
//         }

//         segment *a = nullptr;
//         if (pt.content.back().type == 1)
//             a = &m.letters[m.FindLetter(pt.content.back())];
//         if (pt.content.back().type == 2)
//             a = &m.digits[m.FindDigit(pt.content.back())];
//         if (pt.content.back().type == 3)
//             a = &m.symbols[m.FindSymbol(pt.content.back())];

//         int n = pt.max_indices.back();
//         std::vector<std::string> host_strs(n);

//         #pragma omp parallel for
//         for (int i = 0; i < n; ++i)
//             host_strs[i] = a->ordered_values[i];

//         char* d_suffix_buf;
//         cudaMalloc(&d_suffix_buf, n * MAX_STR_LEN * sizeof(char));
//         char* h_suffix_buf = new char[n * MAX_STR_LEN];

//         #pragma omp parallel for
//         for (int i = 0; i < n; ++i)
//             strncpy(h_suffix_buf + i * MAX_STR_LEN, host_strs[i].c_str(), MAX_STR_LEN);

//         cudaMemcpyAsync(d_suffix_buf, h_suffix_buf, n * MAX_STR_LEN * sizeof(char), cudaMemcpyHostToDevice, stream);

//         char** h_suffix_ptrs = new char*[n];
//         for (int i = 0; i < n; ++i)
//             h_suffix_ptrs[i] = d_suffix_buf + i * MAX_STR_LEN;

//         char** d_suffix_ptrs;
//         cudaMalloc(&d_suffix_ptrs, n * sizeof(char*));
//         cudaMemcpyAsync(d_suffix_ptrs, h_suffix_ptrs, n * sizeof(char*), cudaMemcpyHostToDevice, stream);

//         char* d_prefix;
//         cudaMalloc(&d_prefix, MAX_STR_LEN * sizeof(char));
//         cudaMemcpyAsync(d_prefix, guess.c_str(), MAX_STR_LEN, cudaMemcpyHostToDevice, stream);

//         char* d_output;
//         cudaMalloc(&d_output, n * MAX_STR_LEN * sizeof(char));

//         int blockSize = 256;
//         int gridSize = (n + blockSize - 1) / blockSize;

//         // 异步调用核函数启动器
//         launch_generate_multi_segment_async(d_prefix, d_suffix_ptrs, d_output, n, blockSize, gridSize, stream);

//         cudaStreamSynchronize(stream);

//         std::vector<char> h_output(n * MAX_STR_LEN);
//         cudaMemcpy(h_output.data(), d_output, n * MAX_STR_LEN * sizeof(char), cudaMemcpyDeviceToHost);

//         #pragma omp parallel for
//         for (int i = 0; i < n; ++i) {
//             std::string s(h_output.data() + i * MAX_STR_LEN);
//             if (!s.empty()) {
//                 #pragma omp critical
//                 {
//                     guesses.emplace_back(s);
//                     total_guesses += 1;
//                 }
//             }
//         }

//         delete[] h_suffix_buf;
//         delete[] h_suffix_ptrs;
//         cudaFree(d_suffix_buf);
//         cudaFree(d_suffix_ptrs);
//         cudaFree(d_prefix);
//         cudaFree(d_output);
//     }

//     cudaStreamDestroy(stream);
// }


























//gpu+openmp
void PriorityQueue::Generate(PT pt)
{
    CalProb(pt);

    if (pt.content.size() == 1)
    {
        segment *a = nullptr;
        if (pt.content[0].type == 1)
            a = &m.letters[m.FindLetter(pt.content[0])];
        if (pt.content[0].type == 2)
            a = &m.digits[m.FindDigit(pt.content[0])];
        if (pt.content[0].type == 3)
            a = &m.symbols[m.FindSymbol(pt.content[0])];

        int n = pt.max_indices[0];
        std::vector<std::string> host_strs(n);

        // OpenMP 并行加速生成 host_strs
        #pragma omp parallel for
        for (int i = 0; i < n; ++i)
            host_strs[i] = a->ordered_values[i];

        // --- Flatten all strings to a single char buffer
        char* d_char_buf;
        cudaMalloc(&d_char_buf, n * MAX_STR_LEN * sizeof(char));
        char* h_char_buf = new char[n * MAX_STR_LEN];

        // OpenMP 并行加速拷贝到 char buffer
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            strncpy(h_char_buf + i * MAX_STR_LEN, host_strs[i].c_str(), MAX_STR_LEN);
        }
        cudaMemcpy(d_char_buf, h_char_buf, n * MAX_STR_LEN * sizeof(char), cudaMemcpyHostToDevice);

        // --- Set up device pointer array to each string in d_char_buf
        char** h_ptr_array = new char*[n];
        for (int i = 0; i < n; ++i) {
            h_ptr_array[i] = d_char_buf + i * MAX_STR_LEN;
        }

        char** d_ptr_array;
        cudaMalloc(&d_ptr_array, n * sizeof(char*));
        cudaMemcpy(d_ptr_array, h_ptr_array, n * sizeof(char*), cudaMemcpyHostToDevice);

        char* d_output;
        cudaMalloc(&d_output, n * MAX_STR_LEN * sizeof(char));

        int blockSize = 256;
        int gridSize = (n + blockSize - 1) / blockSize;

        launch_generate_single_segment(d_ptr_array, d_output, n, blockSize, gridSize);

        std::vector<char> h_output(n * MAX_STR_LEN);
        cudaMemcpy(h_output.data(), d_output, n * MAX_STR_LEN * sizeof(char), cudaMemcpyDeviceToHost);

        // OpenMP 并行加速从GPU结果转字符串及保存
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            std::string s(h_output.data() + i * MAX_STR_LEN);
            if (!s.empty()) {
                #pragma omp critical
                {
                    guesses.emplace_back(s);
                    total_guesses += 1;
                }
            }
        }

        delete[] h_char_buf;
        delete[] h_ptr_array;
        cudaFree(d_char_buf);
        cudaFree(d_ptr_array);
        cudaFree(d_output);
    }
    else
    {
        std::string guess;
        int seg_idx = 0;
        for (int idx : pt.curr_indices)
        {
            if (pt.content[seg_idx].type == 1)
                guess += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
            if (pt.content[seg_idx].type == 2)
                guess += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
            if (pt.content[seg_idx].type == 3)
                guess += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];
            seg_idx += 1;
            if (seg_idx == pt.content.size() - 1)
                break;
        }

        segment *a = nullptr;
        if (pt.content.back().type == 1)
            a = &m.letters[m.FindLetter(pt.content.back())];
        if (pt.content.back().type == 2)
            a = &m.digits[m.FindDigit(pt.content.back())];
        if (pt.content.back().type == 3)
            a = &m.symbols[m.FindSymbol(pt.content.back())];

        int n = pt.max_indices.back();
        std::vector<std::string> host_strs(n);

        // OpenMP 并行加速生成 host_strs
        #pragma omp parallel for
        for (int i = 0; i < n; ++i)
            host_strs[i] = a->ordered_values[i];

        char* d_suffix_buf;
        cudaMalloc(&d_suffix_buf, n * MAX_STR_LEN * sizeof(char));
        char* h_suffix_buf = new char[n * MAX_STR_LEN];

        // OpenMP 并行加速拷贝字符串
        #pragma omp parallel for
        for (int i = 0; i < n; ++i)
            strncpy(h_suffix_buf + i * MAX_STR_LEN, host_strs[i].c_str(), MAX_STR_LEN);
        cudaMemcpy(d_suffix_buf, h_suffix_buf, n * MAX_STR_LEN * sizeof(char), cudaMemcpyHostToDevice);

        char** h_suffix_ptrs = new char*[n];
        for (int i = 0; i < n; ++i)
            h_suffix_ptrs[i] = d_suffix_buf + i * MAX_STR_LEN;

        char** d_suffix_ptrs;
        cudaMalloc(&d_suffix_ptrs, n * sizeof(char*));
        cudaMemcpy(d_suffix_ptrs, h_suffix_ptrs, n * sizeof(char*), cudaMemcpyHostToDevice);

        char* d_prefix;
        cudaMalloc(&d_prefix, MAX_STR_LEN * sizeof(char));
        cudaMemcpy(d_prefix, guess.c_str(), MAX_STR_LEN, cudaMemcpyHostToDevice);

        char* d_output;
        cudaMalloc(&d_output, n * MAX_STR_LEN * sizeof(char));

        int blockSize = 256;
        int gridSize = (n + blockSize - 1) / blockSize;

        launch_generate_multi_segment(d_prefix, d_suffix_ptrs, d_output, n, blockSize, gridSize);

        std::vector<char> h_output(n * MAX_STR_LEN);
        cudaMemcpy(h_output.data(), d_output, n * MAX_STR_LEN * sizeof(char), cudaMemcpyDeviceToHost);

        // OpenMP 并行加速从GPU结果转字符串及保存
        #pragma omp parallel for
        for (int i = 0; i < n; ++i) {
            std::string s(h_output.data() + i * MAX_STR_LEN);
            if (!s.empty()) {
                #pragma omp critical
                {
                    guesses.emplace_back(s);
                    total_guesses += 1;
                }
            }
        }

        delete[] h_suffix_buf;
        delete[] h_suffix_ptrs;
        cudaFree(d_suffix_buf);
        cudaFree(d_suffix_ptrs);
        cudaFree(d_prefix);
        cudaFree(d_output);
    }
}









//MPI
// 这个函数是PCFG并行化算法的主要载体
// 尽量看懂，然后进行并行实现
// void PriorityQueue::Generate(PT pt)
// {
//     // 计算PT的概率，这里主要是给PT的概率进行初始化
//     CalProb(pt);
//     int rank = 0, size = 1;
// #ifdef USE_MPI
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);
// #endif
//     // 对于只有一个segment的PT，直接遍历生成其中的所有value即可
//     if (pt.content.size() == 1)
//     {
//         segment *a;
//         if (pt.content[0].type == 1)
//             a = &m.letters[m.FindLetter(pt.content[0])];
//         if (pt.content[0].type == 2)
//             a = &m.digits[m.FindDigit(pt.content[0])];
//         if (pt.content[0].type == 3)
//             a = &m.symbols[m.FindSymbol(pt.content[0])];

//         int max_idx = pt.max_indices[0];
//         int chunk = max_idx / size;
//         int rem = max_idx % size;
//         int start = rank * chunk + std::min(rank, rem);
//         int end = start + chunk + (rank < rem ? 1 : 0);
//         if (end > max_idx) end = max_idx;
//         for (int i = start; i < end; ++i)
//         {
//             string guess = a->ordered_values[i];
//             guesses.emplace_back(guess);
//             total_guesses += 1;
//         }
//     }
//     else
//     {
//         string guess;
//         int seg_idx = 0;
//         for (int idx : pt.curr_indices)
//         {
//             if (pt.content[seg_idx].type == 1)
//                 guess += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
//             if (pt.content[seg_idx].type == 2)
//                 guess += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
//             if (pt.content[seg_idx].type == 3)
//                 guess += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];
//             seg_idx += 1;
//             if (seg_idx == pt.content.size() - 1)
//                 break;
//         }

//         segment *a;
//         if (pt.content[pt.content.size() - 1].type == 1)
//             a = &m.letters[m.FindLetter(pt.content[pt.content.size() - 1])];
//         if (pt.content[pt.content.size() - 1].type == 2)
//             a = &m.digits[m.FindDigit(pt.content[pt.content.size() - 1])];
//         if (pt.content[pt.content.size() - 1].type == 3)
//             a = &m.symbols[m.FindSymbol(pt.content[pt.content.size() - 1])];

//         int max_idx = pt.max_indices[pt.content.size() - 1];
//         int chunk = max_idx / size;
//         int rem = max_idx % size;
//         int start = rank * chunk + std::min(rank, rem);
//         int end = start + chunk + (rank < rem ? 1 : 0);
//         if (end > max_idx) end = max_idx;

//         for (int i = start; i < end; ++i)
//         {
//             string temp = guess + a->ordered_values[i];
//             guesses.emplace_back(temp);
//             total_guesses += 1;
//         }
//     }
// }








//最初始的代码
// void PriorityQueue::Generate(PT pt)
// {
//     // 计算PT的概率，这里主要是给PT的概率进行初始化
//     CalProb(pt);
//     int num_threads = sysconf(_SC_NPROCESSORS_ONLN);
//     // 对于只有一个segment的PT，直接遍历生成其中的所有value即可
//     if (pt.content.size() == 1)
//     {
//         // 指向最后一个segment的指针，这个指针实际指向模型中的统计数据
//         segment *a;
//         // 在模型中定位到这个segment
//         if (pt.content[0].type == 1)
//         {
//             a = &m.letters[m.FindLetter(pt.content[0])];
//         }
//         if (pt.content[0].type == 2)
//         {
//             a = &m.digits[m.FindDigit(pt.content[0])];
//         }
//         if (pt.content[0].type == 3)
//         {
//             a = &m.symbols[m.FindSymbol(pt.content[0])];
//         }

//         // Multi-thread TODO：
//         // 这个for循环就是你需要进行并行化的主要部分了，特别是在多线程&GPU编程任务中
//         // 可以看到，这个循环本质上就是把模型中一个segment的所有value，赋值到PT中，形成一系列新的猜测
//         // 这个过程是可以高度并行化的
//         for (int i = 0; i < pt.max_indices[0]; i += 1)
//         {
//             string guess = a->ordered_values[i];
//             // cout << guess << endl;
//             guesses.emplace_back(guess);
//             total_guesses += 1;
//         }        
//     }
//     else
//     {
//         string guess;
//         int seg_idx = 0;
//         // 这个for循环的作用：给当前PT的所有segment赋予实际的值（最后一个segment除外）
//         // segment值根据curr_indices中对应的值加以确定
//         // 这个for循环你看不懂也没太大问题，并行算法不涉及这里的加速
//         for (int idx : pt.curr_indices)
//         {
//             if (pt.content[seg_idx].type == 1)
//             {
//                 guess += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
//             }
//             if (pt.content[seg_idx].type == 2)
//             {
//                 guess += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
//             }
//             if (pt.content[seg_idx].type == 3)
//             {
//                 guess += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];
//             }
//             seg_idx += 1;
//             if (seg_idx == pt.content.size() - 1)
//             {
//                 break;
//             }
//         }
//         // 指向最后一个segment的指针，这个指针实际指向模型中的统计数据
//         segment *a;
//         if (pt.content[pt.content.size() - 1].type == 1)
//         {
//             a = &m.letters[m.FindLetter(pt.content[pt.content.size() - 1])];
//         }
//         if (pt.content[pt.content.size() - 1].type == 2)
//         {
//             a = &m.digits[m.FindDigit(pt.content[pt.content.size() - 1])];
//         }
//         if (pt.content[pt.content.size() - 1].type == 3)
//         {
//             a = &m.symbols[m.FindSymbol(pt.content[pt.content.size() - 1])];
//         }

//         // Multi-thread TODO：
//         // 这个for循环就是你需要进行并行化的主要部分了，特别是在多线程&GPU编程任务中
//         // 可以看到，这个循环本质上就是把模型中一个segment的所有value，赋值到PT中，形成一系列新的猜测
//         // 这个过程是可以高度并行化的
//         for (int i = 0; i < pt.max_indices[pt.content.size() - 1]; i += 1)
//         {
//             string temp = guess + a->ordered_values[i];
//             // cout << temp << endl;
//             guesses.emplace_back(temp);
//             total_guesses += 1;
//         }

//     }
// }
