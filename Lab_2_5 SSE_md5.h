// md5.h
#ifndef MD5_H
#define MD5_H

#include <iostream>
#include <string>
#include <emmintrin.h>  // SSE2
#include <smmintrin.h>  // SSE4.1

using namespace std;

typedef __m128i bit32x4;
typedef unsigned char Byte;
typedef unsigned int bit32;

// MD5常量定义
#define s11 7
#define s12 12
#define s13 17
#define s14 22
#define s21 5
#define s22 9
#define s23 14
#define s24 20
#define s31 4
#define s32 11
#define s33 16
#define s34 23
#define s41 6
#define s42 10
#define s43 15
#define s44 21
#define GET_LANE0(vec) vgetq_lane_u32(vec, 0)
#define GET_LANE1(vec) vgetq_lane_u32(vec, 1)
#define GET_LANE2(vec) vgetq_lane_u32(vec, 2)
#define GET_LANE3(vec) vgetq_lane_u32(vec, 3)

#define SET_LANE0(val, vec) vsetq_lane_u32(val, vec, 0)
#define SET_LANE1(val, vec) vsetq_lane_u32(val, vec, 1)
#define SET_LANE2(val, vec) vsetq_lane_u32(val, vec, 2)
#define SET_LANE3(val, vec) vsetq_lane_u32(val, vec, 3)

#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))
#define FF(a, b, c, d, x, s, ac) { \
  (a) += F ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
  (a) += G ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
  (a) += H ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
  (a) += I ((b), (c), (d)) + (x) + ac; \
  (a) = ROTATELEFT ((a), (s)); \
  (a) += (b); \
}



// SSE版本的FGHI函数
#define F_SIMD(x, y, z) _mm_or_si128(_mm_and_si128(x, y), _mm_andnot_si128(x, z))
#define G_SIMD(x, y, z) _mm_or_si128(_mm_and_si128(x, z), _mm_andnot_si128(z, y))
#define H_SIMD(x, y, z) _mm_xor_si128(_mm_xor_si128(x, y), z)
#define I_SIMD(x, y, z) _mm_xor_si128(y, _mm_or_si128(x, _mm_andnot_si128(z, _mm_set1_epi32(0xFFFFFFFF))))

// SSE版本的循环左移
#define ROTATELEFT_SIMD(num, n) _mm_or_si128(_mm_slli_epi32(num, n), _mm_srli_epi32(num, 32-(n)))

// SSE版本的轮函数
#define FF_SIMD(a, b, c, d, x, s, ac) { \
  bit32x4 temp = F_SIMD(b, c, d); \
  temp = _mm_add_epi32(temp, x); \
  temp = _mm_add_epi32(temp, _mm_set1_epi32(ac)); \
  a = _mm_add_epi32(a, temp); \
  a = ROTATELEFT_SIMD(a, s); \
  a = _mm_add_epi32(a, b); \
}

#define GG_SIMD(a, b, c, d, x, s, ac) { \
  bit32x4 temp = G_SIMD(b, c, d); \
  temp = _mm_add_epi32(temp, x); \
  temp = _mm_add_epi32(temp, _mm_set1_epi32(ac)); \
  a = _mm_add_epi32(a, temp); \
  a = ROTATELEFT_SIMD(a, s); \
  a = _mm_add_epi32(a, b); \
}

#define HH_SIMD(a, b, c, d, x, s, ac) { \
  bit32x4 temp = H_SIMD(b, c, d); \
  temp = _mm_add_epi32(temp, x); \
  temp = _mm_add_epi32(temp, _mm_set1_epi32(ac)); \
  a = _mm_add_epi32(a, temp); \
  a = ROTATELEFT_SIMD(a, s); \
  a = _mm_add_epi32(a, b); \
}

#define II_SIMD(a, b, c, d, x, s, ac) { \
  bit32x4 temp = I_SIMD(b, c, d); \
  temp = _mm_add_epi32(temp, x); \
  temp = _mm_add_epi32(temp, _mm_set1_epi32(ac)); \
  a = _mm_add_epi32(a, temp); \
  a = ROTATELEFT_SIMD(a, s); \
  a = _mm_add_epi32(a, b); \
}

// 标准MD5函数定义
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))

// 函数声明
void MD5Hash(string input, bit32 *state);
void SIMD_MD5Hash(const string inputs[4], bit32 states[4][4]);

#endif // MD5_H
