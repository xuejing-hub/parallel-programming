// //gpu
// #include <cuda_runtime.h>
// #define MAX_STR_LEN 64

// extern "C" {

// __global__ void generate_single_segment_kernel(char **values, char *output, int count)
// {
//     int idx = blockIdx.x * blockDim.x + threadIdx.x;
//     if (idx >= count) return;

//     char *src = values[idx];
//     char *dst = output + idx * MAX_STR_LEN;

//     int i = 0;
//     for (; i < MAX_STR_LEN - 1 && src[i] != '\0'; ++i)
//         dst[i] = src[i];
//     dst[i] = '\0';
// }

// __global__ void generate_multi_segment_kernel(char *prefix, char **suffixes, char *output, int count)
// {
//     int idx = blockIdx.x * blockDim.x + threadIdx.x;
//     if (idx >= count) return;

//     char *suffix = suffixes[idx];
//     char *dst = output + idx * MAX_STR_LEN;

//     int i = 0;
//     for (; i < MAX_STR_LEN - 1 && prefix[i] != '\0'; ++i)
//         dst[i] = prefix[i];
//     for (int j = 0; i < MAX_STR_LEN - 1 && suffix[j] != '\0' && i < MAX_STR_LEN - 1; ++j, ++i)
//         dst[i] = suffix[j];
//     dst[i] = '\0';
// }

// void launch_generate_single_segment(char **values, char *output, int count, int blockSize, int gridSize) {
//     generate_single_segment_kernel<<<gridSize, blockSize>>>(values, output, count);
//     cudaDeviceSynchronize();
// }

// void launch_generate_multi_segment(char *prefix, char **suffixes, char *output, int count, int blockSize, int gridSize) {
//     generate_multi_segment_kernel<<<gridSize, blockSize>>>(prefix, suffixes, output, count);
//     cudaDeviceSynchronize();
// }

// }





// //gpu+多个PT
// extern "C" {

// // 核函数：将输入字符串数组复制到输出，支持批量所有PT字符串
// __global__ void generate_batch_kernel(char **input_strings, char *output, int count)
// {
//     int idx = blockIdx.x * blockDim.x + threadIdx.x;
//     if (idx >= count) return;

//     char *src = input_strings[idx];
//     char *dst = output + idx * MAX_STR_LEN;

//     int i = 0;
//     // 简单复制字符串，注意末尾'\0'
//     for (; i < MAX_STR_LEN - 1 && src[i] != '\0'; ++i) {
//         dst[i] = src[i];
//     }
//     dst[i] = '\0';
// }

// // 启动接口
// void launch_generate_batch(char **input_strings, char *output, int count, int blockSize, int gridSize) {
//     generate_batch_kernel<<<gridSize, blockSize>>>(input_strings, output, count);
//     cudaDeviceSynchronize();
// }

// }



#include <cuda_runtime.h>
#include <cstring>
#define MAX_STR_LEN 64

extern "C" {

// ========== 核函数部分 ==========

// 单段字符串拷贝核函数
__global__ void kernel_generate_single_segment(char **values, char *output, int count)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= count) return;

    char *src = values[idx];
    char *dst = output + idx * MAX_STR_LEN;

    int i = 0;
    for (; i < MAX_STR_LEN - 1 && src[i] != '\0'; ++i)
        dst[i] = src[i];
    dst[i] = '\0';
}

// 多段拼接（prefix + suffix）核函数
__global__ void kernel_generate_multi_segment(char *prefix, char **suffixes, char *output, int count)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= count) return;

    char *suffix = suffixes[idx];
    char *dst = output + idx * MAX_STR_LEN;

    int i = 0;
    for (; i < MAX_STR_LEN - 1 && prefix[i] != '\0'; ++i)
        dst[i] = prefix[i];
    for (int j = 0; i < MAX_STR_LEN - 1 && suffix[j] != '\0'; ++j, ++i)
        dst[i] = suffix[j];
    dst[i] = '\0';
}

// 批量字符串直接复制核函数
__global__ void kernel_generate_batch(char **input_strings, char *output, int count)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= count) return;

    char *src = input_strings[idx];
    char *dst = output + idx * MAX_STR_LEN;

    int i = 0;
    for (; i < MAX_STR_LEN - 1 && src[i] != '\0'; ++i)
        dst[i] = src[i];
    dst[i] = '\0';
}

} // extern "C"

// ========== 启动函数部分（同步） ==========

// 启动函数：普通函数，不带 <<<>>>，内部调用核函数，且同步等待
extern "C" void launch_generate_single_segment(char **values, char *output, int count, int blockSize, int gridSize) {
    kernel_generate_single_segment<<<gridSize, blockSize>>>(values, output, count);
    cudaDeviceSynchronize();
}

extern "C" void launch_generate_multi_segment(char *prefix, char **suffixes, char *output, int count, int blockSize, int gridSize) {
    kernel_generate_multi_segment<<<gridSize, blockSize>>>(prefix, suffixes, output, count);
    cudaDeviceSynchronize();
}

extern "C" void launch_generate_batch(char **input_strings, char *output, int count, int blockSize, int gridSize) {
    kernel_generate_batch<<<gridSize, blockSize>>>(input_strings, output, count);
    cudaDeviceSynchronize();
}

// ========== 启动函数部分（异步） ==========

// 异步启动函数：普通函数，不带 <<<>>>，内部调用核函数，使用传入的 stream
extern "C" void launch_generate_single_segment_async(char **input, char *output, int count, int blockSize, int gridSize, cudaStream_t stream) {
    kernel_generate_single_segment<<<gridSize, blockSize, 0, stream>>>(input, output, count);
}

extern "C" void launch_generate_multi_segment_async(char *prefix, char **suffixes, char *output, int count, int blockSize, int gridSize, cudaStream_t stream) {
    kernel_generate_multi_segment<<<gridSize, blockSize, 0, stream>>>(prefix, suffixes, output, count);
}

extern "C" void launch_generate_batch_async(char **input_strings, char *output, int count, int blockSize, int gridSize, cudaStream_t stream) {
    kernel_generate_batch<<<gridSize, blockSize, 0, stream>>>(input_strings, output, count);
}
