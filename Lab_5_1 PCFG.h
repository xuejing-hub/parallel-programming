#include <string>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <omp.h>
// #include <chrono>   
// using namespace chrono;

using namespace std;

class segment
{
public:
    int type; // 0: 未设置, 1: 字母, 2: 数字, 3: 特殊字符
    int length; // 长度，例如S6的长度就是6
    segment(int type, int length)
    {
        this->type = type;
        this->length = length;
    };

    // 打印相关信息
    void PrintSeg();

    // 按照概率降序排列的value。例如，123是D3的一个具体value，其概率在D3的所有value中排名第三，那么其位置就是ordered_values[2]
    vector<string> ordered_values;

    // 按照概率降序排列的频数（概率）
    vector<int> ordered_freqs;

    // total_freq作为分母，用于计算每个value的概率
    int total_freq = 0;

    // 未排序的value，其中int就是对应的id
    unordered_map<string, int> values;

    // 根据id，在freqs中查找/修改一个value的频数
    unordered_map<int, int> freqs;


    void insert(string value);
    void order();
    void PrintValues();
};

class PT
{
public:
    // 例如，L6D1的content大小为2，content[0]为L6，content[1]为D1
    vector<segment> content;

    // pivot值，参见PCFG的原理
    int pivot = 0;
    void insert(segment seg);
    void PrintPT();

    // 导出新的PT
    vector<PT> NewPTs();

    // 记录当前每个segment（除了最后一个）对应的value，在模型中的下标
    vector<int> curr_indices;

    // 记录当前每个segment（除了最后一个）对应的value，在模型中的最大下标（即最大可以是max_indices[x]-1）
    vector<int> max_indices;
    // void init();
    float preterm_prob;
    float prob;
};

class model
{
public:
    // 对于PT/LDS而言，序号是递增的
    // 训练时每遇到一个新的PT/LDS，就获取一个新的序号，并且当前序号递增1
    int preterm_id = -1;
    int letters_id = -1;
    int digits_id = -1;
    int symbols_id = -1;
    int GetNextPretermID()
    {
        preterm_id++;
        return preterm_id;
    };
    int GetNextLettersID()
    {
        letters_id++;
        return letters_id;
    };
    int GetNextDigitsID()
    {
        digits_id++;
        return digits_id;
    };
    int GetNextSymbolsID()
    {
        symbols_id++;
        return symbols_id;
    };

    // C++上机和数据结构实验中，一般不允许使用stl
    // 这就导致大家对stl不甚熟悉。现在是时候体会stl的便捷之处了
    // unordered_map: 无序映射
    int total_preterm = 0;
    vector<PT> preterminals;
    int FindPT(PT pt);

    vector<segment> letters;
    vector<segment> digits;
    vector<segment> symbols;
    int FindLetter(segment seg);
    int FindDigit(segment seg);
    int FindSymbol(segment seg);

    unordered_map<int, int> preterm_freq;
    unordered_map<int, int> letters_freq;
    unordered_map<int, int> digits_freq;
    unordered_map<int, int> symbols_freq;

    vector<PT> ordered_pts;

    // 给定一个训练集，对模型进行训练
    void train(string train_path);

    // 对已经训练的模型进行保存
    void store(string store_path);

    // 从现有的模型文件中加载模型
    void load(string load_path);

    // 对一个给定的口令进行切分
    void parse(string pw);

    void order();

    // 打印模型
    void print();
};

// 优先队列，用于按照概率降序生成口令猜测
// 实际上，这个class负责队列维护、口令生成、结果存储的全部过程
//初始代码，单个PT
// class PriorityQueue
// {
// public:
//     // 用vector实现的priority queue
//     vector<PT> priority;

//     // 模型作为成员，辅助猜测生成
//     model m;

//     // 计算一个pt的概率
//     void CalProb(PT &pt);

//     // 优先队列的初始化
//     void init();

//     // 对优先队列的一个PT，生成所有guesses
//     void Generate(PT pt);

//     // 将优先队列最前面的一个PT
//     void PopNext();
//     int total_guesses = 0;
//     vector<string> guesses;
// };


//MPI的多个PT
class PriorityQueue
{
public:
    // 用vector实现的priority queue
    vector<PT> priority;

    // 模型作为成员，辅助猜测生成
    model m;

    // 计算一个pt的概率
    void CalProb(PT &pt);

    // 优先队列的初始化
    void init();

    // 对优先队列的一个PT，生成所有guesses
    void Generate(PT pt);

    // 将优先队列最前面的一个PT
    void PopNext();
    int total_guesses = 0;
    vector<string> guesses;

    // 批量弹出和批量生成
    void PopBatchAndGenerate(int batch_size)
    {
        vector<PT> batch;
        // 1. 批量弹出
        for (int i = 0; i < batch_size && !priority.empty(); ++i) {
            batch.push_back(priority.back());
            priority.pop_back();
            }
        // 2. 对每个PT生成口令
        for (auto& pt : batch) {
            Generate(pt);
        }
        // 3. 新PT全部放回队列
        for (auto& pt : batch) {
            vector<PT> new_pts = pt.NewPTs();
            for (auto& npt : new_pts) {
                priority.push_back(npt);
            }
        }
    }
};





// //gpu的多PT
// #include <cstring>          // for strncpy
// #include <cuda_runtime.h>   // for cudaMalloc, cudaMemcpy, cudaFree, cudaMemcpyHostToDevice等
// #define MAX_STR_LEN 64
// extern "C" void launch_generate_batch(char **input_strings, char *output, int count, int blockSize, int gridSize);

// class PriorityQueue
// {
// public:
//     vector<PT> priority;
//     model m;

//     void CalProb(PT &pt);

//     void init();

//     // 原来单PT生成
//     void Generate(PT pt);

//     void PopNext();

//     int total_guesses = 0;
//     vector<string> guesses;

//     // 新增：批量PT生成到GPU
//     void GenerateBatch(const vector<PT>& pts)
//     {
//         // 1. 计算概率（可以并行）
//         for (auto& pt : pts) {
//             CalProb(const_cast<PT&>(pt));  // 如果CalProb非const，先转换
//         }

//         // 2. 准备GPU数据结构
//         // 这里需要整理所有pt的内容到GPU缓冲
//         // 先计算总字符串数和总长度等

//         int total_strings = 0;
//         vector<int> pt_sizes;
//         for (auto& pt : pts) {
//             int n = (pt.content.size() == 1) ? pt.max_indices[0] : pt.max_indices.back();
//             pt_sizes.push_back(n);
//             total_strings += n;
//         }

//         // 举例：把所有PT的字符串集中到一个大数组，扁平化

//         vector<string> all_strings;
//         vector<int> pt_offsets; // 记录每个PT在all_strings的起始索引
//         int offset = 0;

//         for (size_t pi = 0; pi < pts.size(); ++pi)
//         {
//             const PT& pt = pts[pi];
//             segment* a = nullptr;
//             if (pt.content.size() == 1)
//             {
//                 if (pt.content[0].type == 1)
//                     a = &m.letters[m.FindLetter(pt.content[0])];
//                 else if (pt.content[0].type == 2)
//                     a = &m.digits[m.FindDigit(pt.content[0])];
//                 else if (pt.content[0].type == 3)
//                     a = &m.symbols[m.FindSymbol(pt.content[0])];

//                 for (int i = 0; i < pt.max_indices[0]; ++i)
//                     all_strings.push_back(a->ordered_values[i]);
//             }
//             else
//             {
//                 // 多段情况，先构造前缀
//                 string guess;
//                 int seg_idx = 0;
//                 for (int idx : pt.curr_indices)
//                 {
//                     if (pt.content[seg_idx].type == 1)
//                         guess += m.letters[m.FindLetter(pt.content[seg_idx])].ordered_values[idx];
//                     else if (pt.content[seg_idx].type == 2)
//                         guess += m.digits[m.FindDigit(pt.content[seg_idx])].ordered_values[idx];
//                     else if (pt.content[seg_idx].type == 3)
//                         guess += m.symbols[m.FindSymbol(pt.content[seg_idx])].ordered_values[idx];

//                     seg_idx++;
//                     if (seg_idx == pt.content.size() - 1)
//                         break;
//                 }
//                 a = nullptr;
//                 if (pt.content.back().type == 1)
//                     a = &m.letters[m.FindLetter(pt.content.back())];
//                 else if (pt.content.back().type == 2)
//                     a = &m.digits[m.FindDigit(pt.content.back())];
//                 else if (pt.content.back().type == 3)
//                     a = &m.symbols[m.FindSymbol(pt.content.back())];

//                 // 将前缀和后缀字符串拼接形成最终all_strings
//                 for (int i = 0; i < pt.max_indices.back(); ++i)
//                     all_strings.push_back(guess + a->ordered_values[i]);
//             }

//             pt_offsets.push_back(offset);
//             offset += pt_sizes[pi];
//         }

//         // 3. 把all_strings扁平化，拷贝到GPU（同单PT逻辑）

//         int total_chars = total_strings * MAX_STR_LEN;
//         char* h_char_buf = new char[total_chars];
//         for (int i = 0; i < total_strings; ++i) {
//             strncpy(h_char_buf + i * MAX_STR_LEN, all_strings[i].c_str(), MAX_STR_LEN);
//         }

//         char* d_char_buf;
//         cudaMalloc(&d_char_buf, total_chars * sizeof(char));
//         cudaMemcpy(d_char_buf, h_char_buf, total_chars * sizeof(char), cudaMemcpyHostToDevice);

//         // 4. 准备指针数组指向每个字符串
//         char** h_ptr_array = new char*[total_strings];
//         for (int i = 0; i < total_strings; ++i)
//             h_ptr_array[i] = d_char_buf + i * MAX_STR_LEN;

//         char** d_ptr_array;
//         cudaMalloc(&d_ptr_array, total_strings * sizeof(char*));
//         cudaMemcpy(d_ptr_array, h_ptr_array, total_strings * sizeof(char*), cudaMemcpyHostToDevice);

//         // 5. 输出缓冲区
//         char* d_output;
//         cudaMalloc(&d_output, total_chars * sizeof(char));

//         // 6. 调用新的核函数（需你实现）
//         int blockSize = 256;
//         int gridSize = (total_strings + blockSize - 1) / blockSize;
//         launch_generate_batch(d_ptr_array, d_output, total_strings, blockSize, gridSize);

//         // 7. 拷回结果，拆分成各PT的guess
//         std::vector<char> h_output(total_chars);
//         cudaMemcpy(h_output.data(), d_output, total_chars * sizeof(char), cudaMemcpyDeviceToHost);

//         int guess_idx = 0;
//         for (size_t pi = 0; pi < pts.size(); ++pi)
//         {
//             for (int i = 0; i < pt_sizes[pi]; ++i)
//             {
//                 std::string s(h_output.data() + guess_idx * MAX_STR_LEN);
//                 if (!s.empty()) guesses.emplace_back(s);
//                 total_guesses++;
//                 guess_idx++;
//             }
//         }

//         // 8. 释放资源
//         delete[] h_char_buf;
//         delete[] h_ptr_array;
//         cudaFree(d_char_buf);
//         cudaFree(d_ptr_array);
//         cudaFree(d_output);
//     }

//     // 修改PopBatchAndGenerate调用GenerateBatch
//     void PopBatchAndGenerate(int batch_size)
//     {
//         vector<PT> batch;
//         for (int i = 0; i < batch_size && !priority.empty(); ++i) {
//             batch.push_back(priority.back());
//             priority.pop_back();
//         }
//         GenerateBatch(batch);

//         for (auto& pt : batch) {
//             vector<PT> new_pts = pt.NewPTs();
//             for (auto& npt : new_pts) {
//                 priority.push_back(npt);
//             }
//         }
//     }
// };
