// md5.cpp
#include "md5.h"
#include <iomanip>
#include <assert.h>
#include <chrono>
#include <cstring>

using namespace std;
using namespace chrono;

Byte* StringProcess(string input, int* n_byte) {
    // 将输入的字符串转换为Byte为单位的数组
    Byte *blocks = (Byte *)input.c_str();
    int length = input.length();

    // 计算原始消息长度（以比特为单位）
    int bitLength = length * 8;

    // paddingBits: 原始消息需要的padding长度（以bit为单位）
    int paddingBits = bitLength % 512;
    if (paddingBits > 448)
    {
        paddingBits = 512 - (paddingBits - 448);
    }
    else if (paddingBits < 448)
    {
        paddingBits = 448 - paddingBits;
    }
    else if (paddingBits == 448)
    {
        paddingBits = 512;
    }

    // 原始消息需要的padding长度（以Byte为单位）
    int paddingBytes = paddingBits / 8;

    // 创建最终的字节数组
    int paddedLength = length + paddingBytes + 8;
    Byte *paddedMessage = new Byte[paddedLength];

    // 复制原始消息
    memcpy(paddedMessage, blocks, length);

    // 添加填充字节。填充时，第一位为1，后面的所有位均为0。
    paddedMessage[length] = 0x80;
    memset(paddedMessage + length + 1, 0, paddingBytes - 1); 

    // 添加消息长度（64比特，小端格式）
    for (int i = 0; i < 8; ++i)
    {
        paddedMessage[length + paddingBytes + i] = ((uint64_t)length * 8 >> (i * 8)) & 0xFF;
    }

    *n_byte = paddedLength;
    return paddedMessage;
}

void SIMD_MD5Hash(const std::string inputs[4], bit32 states[4][4]) {
    // 初始化MD5状态向量
    bit32x4 state[4] = {
        _mm_set1_epi32(0x67452301),
        _mm_set1_epi32(0xefcdab89),
        _mm_set1_epi32(0x98badcfe),
        _mm_set1_epi32(0x10325476)
    };

    // 预处理4个输入
    Byte* paddedMessages[4];
    int messageLengths[4];
    int maxBlocks = 0;

    for(int i = 0; i < 4; i++) {
        paddedMessages[i] = StringProcess(inputs[i], &messageLengths[i]);
        int blocks = messageLengths[i] / 64;
        maxBlocks = max(maxBlocks, blocks);
    }

    // 处理所有块
    for(int block = 0; block < maxBlocks; block++) {
        bit32x4 x[16];
        
        // 加载16个32位字到x数组
        for(int j = 0; j < 16; j++) {
            uint32_t values[4] = {0, 0, 0, 0};
            for(int i = 0; i < 4; i++) {
                if(block * 64 < messageLengths[i]) {
                    int offset = block * 64 + j * 4;
                    values[i] = ((uint32_t)paddedMessages[i][offset]) |
                               ((uint32_t)paddedMessages[i][offset + 1] << 8) |
                               ((uint32_t)paddedMessages[i][offset + 2] << 16) |
                               ((uint32_t)paddedMessages[i][offset + 3] << 24);
                }
            }
            x[j] = _mm_loadu_si128((const __m128i*)values);
        }

        // 保存当前状态
        bit32x4 a = state[0];
        bit32x4 b = state[1];
        bit32x4 c = state[2];
        bit32x4 d = state[3];

        // Round 1
        FF_SIMD(a, b, c, d, x[ 0], s11, 0xd76aa478);
        FF_SIMD(d, a, b, c, x[ 1], s12, 0xe8c7b756);
        FF_SIMD(c, d, a, b, x[ 2], s13, 0x242070db);
        FF_SIMD(b, c, d, a, x[ 3], s14, 0xc1bdceee);
        FF_SIMD(a, b, c, d, x[ 4], s11, 0xf57c0faf);
        FF_SIMD(d, a, b, c, x[ 5], s12, 0x4787c62a);
        FF_SIMD(c, d, a, b, x[ 6], s13, 0xa8304613);
        FF_SIMD(b, c, d, a, x[ 7], s14, 0xfd469501);
        FF_SIMD(a, b, c, d, x[ 8], s11, 0x698098d8);
        FF_SIMD(d, a, b, c, x[ 9], s12, 0x8b44f7af);
        FF_SIMD(c, d, a, b, x[10], s13, 0xffff5bb1);
        FF_SIMD(b, c, d, a, x[11], s14, 0x895cd7be);
        FF_SIMD(a, b, c, d, x[12], s11, 0x6b901122);
        FF_SIMD(d, a, b, c, x[13], s12, 0xfd987193);
        FF_SIMD(c, d, a, b, x[14], s13, 0xa679438e);
        FF_SIMD(b, c, d, a, x[15], s14, 0x49b40821);

        // Round 2
        GG_SIMD(a, b, c, d, x[ 1], s21, 0xf61e2562);
        GG_SIMD(d, a, b, c, x[ 6], s22, 0xc040b340);
        GG_SIMD(c, d, a, b, x[11], s23, 0x265e5a51);
        GG_SIMD(b, c, d, a, x[ 0], s24, 0xe9b6c7aa);
        GG_SIMD(a, b, c, d, x[ 5], s21, 0xd62f105d);
        GG_SIMD(d, a, b, c, x[10], s22, 0x02441453);
        GG_SIMD(c, d, a, b, x[15], s23, 0xd8a1e681);
        GG_SIMD(b, c, d, a, x[ 4], s24, 0xe7d3fbc8);
        GG_SIMD(a, b, c, d, x[ 9], s21, 0x21e1cde6);
        GG_SIMD(d, a, b, c, x[14], s22, 0xc33707d6);
        GG_SIMD(c, d, a, b, x[ 3], s23, 0xf4d50d87);
        GG_SIMD(b, c, d, a, x[ 8], s24, 0x455a14ed);
        GG_SIMD(a, b, c, d, x[13], s21, 0xa9e3e905);
        GG_SIMD(d, a, b, c, x[ 2], s22, 0xfcefa3f8);
        GG_SIMD(c, d, a, b, x[ 7], s23, 0x676f02d9);
        GG_SIMD(b, c, d, a, x[12], s24, 0x8d2a4c8a);

        // Round 3
        HH_SIMD(a, b, c, d, x[ 5], s31, 0xfffa3942);
        HH_SIMD(d, a, b, c, x[ 8], s32, 0x8771f681);
        HH_SIMD(c, d, a, b, x[11], s33, 0x6d9d6122);
        HH_SIMD(b, c, d, a, x[14], s34, 0xfde5380c);
        HH_SIMD(a, b, c, d, x[ 1], s31, 0xa4beea44);
        HH_SIMD(d, a, b, c, x[ 4], s32, 0x4bdecfa9);
        HH_SIMD(c, d, a, b, x[ 7], s33, 0xf6bb4b60);
        HH_SIMD(b, c, d, a, x[10], s34, 0xbebfbc70);
        HH_SIMD(a, b, c, d, x[13], s31, 0x289b7ec6);
        HH_SIMD(d, a, b, c, x[ 0], s32, 0xeaa127fa);
        HH_SIMD(c, d, a, b, x[ 3], s33, 0xd4ef3085);
        HH_SIMD(b, c, d, a, x[ 6], s34, 0x04881d05);
        HH_SIMD(a, b, c, d, x[ 9], s31, 0xd9d4d039);
        HH_SIMD(d, a, b, c, x[12], s32, 0xe6db99e5);
        HH_SIMD(c, d, a, b, x[15], s33, 0x1fa27cf8);
        HH_SIMD(b, c, d, a, x[ 2], s34, 0xc4ac5665);

        // Round 4
        II_SIMD(a, b, c, d, x[ 0], s41, 0xf4292244);
        II_SIMD(d, a, b, c, x[ 7], s42, 0x432aff97);
        II_SIMD(c, d, a, b, x[14], s43, 0xab9423a7);
        II_SIMD(b, c, d, a, x[ 5], s44, 0xfc93a039);
        II_SIMD(a, b, c, d, x[12], s41, 0x655b59c3);
        II_SIMD(d, a, b, c, x[ 3], s42, 0x8f0ccc92);
        II_SIMD(c, d, a, b, x[10], s43, 0xffeff47d);
        II_SIMD(b, c, d, a, x[ 1], s44, 0x85845dd1);
        II_SIMD(a, b, c, d, x[ 8], s41, 0x6fa87e4f);
        II_SIMD(d, a, b, c, x[15], s42, 0xfe2ce6e0);
        II_SIMD(c, d, a, b, x[ 6], s43, 0xa3014314);
        II_SIMD(b, c, d, a, x[13], s44, 0x4e0811a1);
        II_SIMD(a, b, c, d, x[ 4], s41, 0xf7537e82);
        II_SIMD(d, a, b, c, x[11], s42, 0xbd3af235);
        II_SIMD(c, d, a, b, x[ 2], s43, 0x2ad7d2bb);
        II_SIMD(b, c, d, a, x[ 9], s44, 0xeb86d391);

        // 更新状态
        state[0] = _mm_add_epi32(state[0], a);
        state[1] = _mm_add_epi32(state[1], b);
        state[2] = _mm_add_epi32(state[2], c);
        state[3] = _mm_add_epi32(state[3], d);
    }

    // 提取结果并调整字节序
    alignas(16) uint32_t tmp[4];
    for(int i = 0; i < 4; i++) {
        _mm_store_si128((__m128i*)tmp, state[i]);
        for(int j = 0; j < 4; j++) {
            uint32_t value = tmp[j];
            states[j][i] = ((value & 0xff) << 24) |
                          ((value & 0xff00) << 8) |
                          ((value & 0xff0000) >> 8) |
                          ((value & 0xff000000) >> 24);
        }
    }

    // 清理内存
    for(int i = 0; i < 4; i++) {
        delete[] paddedMessages[i];
    }
}

// 标准MD5Hash函数保持不变
void MD5Hash(string input, bit32 *state)
{

	Byte *paddedMessage;
	int *messageLength = new int[1];
	for (int i = 0; i < 1; i += 1)
	{
		paddedMessage = StringProcess(input, &messageLength[i]);
		// cout<<messageLength[i]<<endl;
		assert(messageLength[i] == messageLength[0]);
	}
	int n_blocks = messageLength[0] / 64;

	// bit32* state= new bit32[4];
	state[0] = 0x67452301;
	state[1] = 0xefcdab89;
	state[2] = 0x98badcfe;
	state[3] = 0x10325476;

	// 逐block地更新state
	for (int i = 0; i < n_blocks; i += 1)
	{
		bit32 x[16];

		// 下面的处理，在理解上较为复杂
		for (int i1 = 0; i1 < 16; ++i1)
		{
			x[i1] = (paddedMessage[4 * i1 + i * 64]) |
					(paddedMessage[4 * i1 + 1 + i * 64] << 8) |
					(paddedMessage[4 * i1 + 2 + i * 64] << 16) |
					(paddedMessage[4 * i1 + 3 + i * 64] << 24);
		}

		bit32 a = state[0], b = state[1], c = state[2], d = state[3];

		auto start = system_clock::now();
		/* Round 1 */
		FF(a, b, c, d, x[0], s11, 0xd76aa478);
		FF(d, a, b, c, x[1], s12, 0xe8c7b756);
		FF(c, d, a, b, x[2], s13, 0x242070db);
		FF(b, c, d, a, x[3], s14, 0xc1bdceee);
		FF(a, b, c, d, x[4], s11, 0xf57c0faf);
		FF(d, a, b, c, x[5], s12, 0x4787c62a);
		FF(c, d, a, b, x[6], s13, 0xa8304613);
		FF(b, c, d, a, x[7], s14, 0xfd469501);
		FF(a, b, c, d, x[8], s11, 0x698098d8);
		FF(d, a, b, c, x[9], s12, 0x8b44f7af);
		FF(c, d, a, b, x[10], s13, 0xffff5bb1);
		FF(b, c, d, a, x[11], s14, 0x895cd7be);
		FF(a, b, c, d, x[12], s11, 0x6b901122);
		FF(d, a, b, c, x[13], s12, 0xfd987193);
		FF(c, d, a, b, x[14], s13, 0xa679438e);
		FF(b, c, d, a, x[15], s14, 0x49b40821);

		/* Round 2 */
		GG(a, b, c, d, x[1], s21, 0xf61e2562);
		GG(d, a, b, c, x[6], s22, 0xc040b340);
		GG(c, d, a, b, x[11], s23, 0x265e5a51);
		GG(b, c, d, a, x[0], s24, 0xe9b6c7aa);
		GG(a, b, c, d, x[5], s21, 0xd62f105d);
		GG(d, a, b, c, x[10], s22, 0x2441453);
		GG(c, d, a, b, x[15], s23, 0xd8a1e681);
		GG(b, c, d, a, x[4], s24, 0xe7d3fbc8);
		GG(a, b, c, d, x[9], s21, 0x21e1cde6);
		GG(d, a, b, c, x[14], s22, 0xc33707d6);
		GG(c, d, a, b, x[3], s23, 0xf4d50d87);
		GG(b, c, d, a, x[8], s24, 0x455a14ed);
		GG(a, b, c, d, x[13], s21, 0xa9e3e905);
		GG(d, a, b, c, x[2], s22, 0xfcefa3f8);
		GG(c, d, a, b, x[7], s23, 0x676f02d9);
		GG(b, c, d, a, x[12], s24, 0x8d2a4c8a);

		/* Round 3 */
		HH(a, b, c, d, x[5], s31, 0xfffa3942);
		HH(d, a, b, c, x[8], s32, 0x8771f681);
		HH(c, d, a, b, x[11], s33, 0x6d9d6122);
		HH(b, c, d, a, x[14], s34, 0xfde5380c);
		HH(a, b, c, d, x[1], s31, 0xa4beea44);
		HH(d, a, b, c, x[4], s32, 0x4bdecfa9);
		HH(c, d, a, b, x[7], s33, 0xf6bb4b60);
		HH(b, c, d, a, x[10], s34, 0xbebfbc70);
		HH(a, b, c, d, x[13], s31, 0x289b7ec6);
		HH(d, a, b, c, x[0], s32, 0xeaa127fa);
		HH(c, d, a, b, x[3], s33, 0xd4ef3085);
		HH(b, c, d, a, x[6], s34, 0x4881d05);
		HH(a, b, c, d, x[9], s31, 0xd9d4d039);
		HH(d, a, b, c, x[12], s32, 0xe6db99e5);
		HH(c, d, a, b, x[15], s33, 0x1fa27cf8);
		HH(b, c, d, a, x[2], s34, 0xc4ac5665);

		/* Round 4 */
		II(a, b, c, d, x[0], s41, 0xf4292244);
		II(d, a, b, c, x[7], s42, 0x432aff97);
		II(c, d, a, b, x[14], s43, 0xab9423a7);
		II(b, c, d, a, x[5], s44, 0xfc93a039);
		II(a, b, c, d, x[12], s41, 0x655b59c3);
		II(d, a, b, c, x[3], s42, 0x8f0ccc92);
		II(c, d, a, b, x[10], s43, 0xffeff47d);
		II(b, c, d, a, x[1], s44, 0x85845dd1);
		II(a, b, c, d, x[8], s41, 0x6fa87e4f);
		II(d, a, b, c, x[15], s42, 0xfe2ce6e0);
		II(c, d, a, b, x[6], s43, 0xa3014314);
		II(b, c, d, a, x[13], s44, 0x4e0811a1);
		II(a, b, c, d, x[4], s41, 0xf7537e82);
		II(d, a, b, c, x[11], s42, 0xbd3af235);
		II(c, d, a, b, x[2], s43, 0x2ad7d2bb);
		II(b, c, d, a, x[9], s44, 0xeb86d391);

		state[0] += a;
		state[1] += b;
		state[2] += c;
		state[3] += d;
	}

	// 下面的处理，在理解上较为复杂
	for (int i = 0; i < 4; i++)
	{
		uint32_t value = state[i];
		state[i] = ((value & 0xff) << 24) |		 // 将最低字节移到最高位
				   ((value & 0xff00) << 8) |	 // 将次低字节左移
				   ((value & 0xff0000) >> 8) |	 // 将次高字节右移
				   ((value & 0xff000000) >> 24); // 将最高字节移到最低位
	}

	// 输出最终的hash结果
	// for (int i1 = 0; i1 < 4; i1 += 1)
	// {
	// 	cout << std::setw(8) << std::setfill('0') << hex << state[i1];
	// }
	// cout << endl;

	// 释放动态分配的内存
	// 实现SIMD并行算法的时候，也请记得及时回收内存！
	delete[] paddedMessage;
	delete[] messageLength;
}
