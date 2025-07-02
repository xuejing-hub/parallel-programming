//MPI之后的代码
// #include <iostream>
// #include <string>
// // #include <arm_neon.h>

// using namespace std;

// typedef uint32x4_t bit32x4;
// typedef unsigned char Byte;
// typedef unsigned int bit32;

// #define s11 7
// #define s12 12
// #define s13 17
// #define s14 22
// #define s21 5
// #define s22 9
// #define s23 14
// #define s24 20
// #define s31 4
// #define s32 11
// #define s33 16
// #define s34 23
// #define s41 6
// #define s42 10
// #define s43 15
// #define s44 21


// #define GET_LANE0(vec) vgetq_lane_u32(vec, 0)
// #define GET_LANE1(vec) vgetq_lane_u32(vec, 1)
// #define GET_LANE2(vec) vgetq_lane_u32(vec, 2)
// #define GET_LANE3(vec) vgetq_lane_u32(vec, 3)

// #define SET_LANE0(val, vec) vsetq_lane_u32(val, vec, 0)
// #define SET_LANE1(val, vec) vsetq_lane_u32(val, vec, 1)
// #define SET_LANE2(val, vec) vsetq_lane_u32(val, vec, 2)
// #define SET_LANE3(val, vec) vsetq_lane_u32(val, vec, 3)

// #define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
// #define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
// #define H(x, y, z) ((x) ^ (y) ^ (z))
// #define I(x, y, z) ((y) ^ ((x) | (~z)))

// #define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))
// #define FF(a, b, c, d, x, s, ac) { \
//   (a) += F ((b), (c), (d)) + (x) + ac; \
//   (a) = ROTATELEFT ((a), (s)); \
//   (a) += (b); \
// }
// #define GG(a, b, c, d, x, s, ac) { \
//   (a) += G ((b), (c), (d)) + (x) + ac; \
//   (a) = ROTATELEFT ((a), (s)); \
//   (a) += (b); \
// }
// #define HH(a, b, c, d, x, s, ac) { \
//   (a) += H ((b), (c), (d)) + (x) + ac; \
//   (a) = ROTATELEFT ((a), (s)); \
//   (a) += (b); \
// }
// #define II(a, b, c, d, x, s, ac) { \
//   (a) += I ((b), (c), (d)) + (x) + ac; \
//   (a) = ROTATELEFT ((a), (s)); \
//   (a) += (b); \
// }

// #define ROTL_SIMD(x, n) (vorrq_u32(vshlq_n_u32((x), (n)), vshrq_n_u32((x), (32 - (n)))))

// #define F_SIMD(x, y, z) vorrq_u32(vandq_u32((x), (y)), vandq_u32(vmvnq_u32((x)), (z)))
// #define G_SIMD(x, y, z) vorrq_u32(vandq_u32((x), (z)), vandq_u32((y), vmvnq_u32((z))))
// #define H_SIMD(x, y, z) veorq_u32(veorq_u32((x), (y)), (z))
// #define I_SIMD(x, y, z) veorq_u32((y), vorrq_u32((x), vmvnq_u32((z))))

// #define FF_SIMD(a, b, c, d, m, s, ti) \
//     { \
//         bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
//         _tmp = vaddq_u32(F_SIMD((b), (c), (d)), _tmp); \
//         (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
//     }

// #define GG_SIMD(a, b, c, d, m, s, ti) \
//     { \
//         bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
//         _tmp = vaddq_u32(G_SIMD((b), (c), (d)), _tmp); \
//         (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
//     }

// #define HH_SIMD(a, b, c, d, m, s, ti) \
//     { \
//         bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
//         _tmp = vaddq_u32(H_SIMD((b), (c), (d)), _tmp); \
//         (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
//     }

// #define II_SIMD(a, b, c, d, m, s, ti) \
//     { \
//         bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
//         _tmp = vaddq_u32(I_SIMD((b), (c), (d)), _tmp); \
//         (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
//     }

// extern const bit32 T[64];
// extern const int shifts[64];
// void MD5Hash(string input, bit32 *state);
// void SIMD_MD5Hash(const string inputs[4], bit32 states[4][4]);















//最初始的代码
// #include <iostream>
// #include <string>
// // #include <arm_neon.h>

// using namespace std;

// typedef uint32x4_t bit32x4;
// typedef unsigned char Byte;
// typedef unsigned int bit32;

// #define s11 7
// #define s12 12
// #define s13 17
// #define s14 22
// #define s21 5
// #define s22 9
// #define s23 14
// #define s24 20
// #define s31 4
// #define s32 11
// #define s33 16
// #define s34 23
// #define s41 6
// #define s42 10
// #define s43 15
// #define s44 21


// #define GET_LANE0(vec) vgetq_lane_u32(vec, 0)
// #define GET_LANE1(vec) vgetq_lane_u32(vec, 1)
// #define GET_LANE2(vec) vgetq_lane_u32(vec, 2)
// #define GET_LANE3(vec) vgetq_lane_u32(vec, 3)

// #define SET_LANE0(val, vec) vsetq_lane_u32(val, vec, 0)
// #define SET_LANE1(val, vec) vsetq_lane_u32(val, vec, 1)
// #define SET_LANE2(val, vec) vsetq_lane_u32(val, vec, 2)
// #define SET_LANE3(val, vec) vsetq_lane_u32(val, vec, 3)

// #define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
// #define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
// #define H(x, y, z) ((x) ^ (y) ^ (z))
// #define I(x, y, z) ((y) ^ ((x) | (~z)))

// #define ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))
// #define FF(a, b, c, d, x, s, ac) { \
//   (a) += F ((b), (c), (d)) + (x) + ac; \
//   (a) = ROTATELEFT ((a), (s)); \
//   (a) += (b); \
// }
// #define GG(a, b, c, d, x, s, ac) { \
//   (a) += G ((b), (c), (d)) + (x) + ac; \
//   (a) = ROTATELEFT ((a), (s)); \
//   (a) += (b); \
// }
// #define HH(a, b, c, d, x, s, ac) { \
//   (a) += H ((b), (c), (d)) + (x) + ac; \
//   (a) = ROTATELEFT ((a), (s)); \
//   (a) += (b); \
// }
// #define II(a, b, c, d, x, s, ac) { \
//   (a) += I ((b), (c), (d)) + (x) + ac; \
//   (a) = ROTATELEFT ((a), (s)); \
//   (a) += (b); \
// }

// #define ROTL_SIMD(x, n) (vorrq_u32(vshlq_n_u32((x), (n)), vshrq_n_u32((x), (32 - (n)))))

// #define F_SIMD(x, y, z) vorrq_u32(vandq_u32((x), (y)), vandq_u32(vmvnq_u32((x)), (z)))
// #define G_SIMD(x, y, z) vorrq_u32(vandq_u32((x), (z)), vandq_u32((y), vmvnq_u32((z))))
// #define H_SIMD(x, y, z) veorq_u32(veorq_u32((x), (y)), (z))
// #define I_SIMD(x, y, z) veorq_u32((y), vorrq_u32((x), vmvnq_u32((z))))

// #define FF_SIMD(a, b, c, d, m, s, ti) \
//     { \
//         bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
//         _tmp = vaddq_u32(F_SIMD((b), (c), (d)), _tmp); \
//         (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
//     }

// #define GG_SIMD(a, b, c, d, m, s, ti) \
//     { \
//         bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
//         _tmp = vaddq_u32(G_SIMD((b), (c), (d)), _tmp); \
//         (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
//     }

// #define HH_SIMD(a, b, c, d, m, s, ti) \
//     { \
//         bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
//         _tmp = vaddq_u32(H_SIMD((b), (c), (d)), _tmp); \
//         (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
//     }

// #define II_SIMD(a, b, c, d, m, s, ti) \
//     { \
//         bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
//         _tmp = vaddq_u32(I_SIMD((b), (c), (d)), _tmp); \
//         (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
//     }

// extern const bit32 T[64];
// extern const int shifts[64];
// void MD5Hash(string input, bit32 *state);
// void SIMD_MD5Hash(const string inputs[4], bit32 states[4][4]);














//gpu
#include <cuda_runtime.h>
#include <string>

typedef unsigned char Byte;
typedef unsigned int bit32;
typedef uint4 bit32x4;  // CUDA内置4x32bit向量

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

// 取单个分量
#define GET_LANE0(vec) ((vec).x)
#define GET_LANE1(vec) ((vec).y)
#define GET_LANE2(vec) ((vec).z)
#define GET_LANE3(vec) ((vec).w)

// 设单个分量
#define SET_LANE0(val, vec) (vec).x = (val)
#define SET_LANE1(val, vec) (vec).y = (val)
#define SET_LANE2(val, vec) (vec).z = (val)
#define SET_LANE3(val, vec) (vec).w = (val)

// 标量MD5基本函数
#define F(x, y, z) (((x) & (y)) | ((~(x)) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~(z))))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~(z))))

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

// CUDA版本向量左旋转
__device__ __host__ inline bit32x4 ROTL_SIMD(bit32x4 x, int n) {
    bit32x4 left = make_uint4(x.x << n, x.y << n, x.z << n, x.w << n);
    bit32x4 right = make_uint4(x.x >> (32 - n), x.y >> (32 - n), x.z >> (32 - n), x.w >> (32 - n));
    return make_uint4(left.x | right.x, left.y | right.y, left.z | right.z, left.w | right.w);
}

// CUDA版本F/G/H/I函数，逐分量计算
__device__ __host__ inline bit32x4 F_SIMD(bit32x4 x, bit32x4 y, bit32x4 z) {
    return make_uint4(
        (x.x & y.x) | (~x.x & z.x),
        (x.y & y.y) | (~x.y & z.y),
        (x.z & y.z) | (~x.z & z.z),
        (x.w & y.w) | (~x.w & z.w));
}
__device__ __host__ inline bit32x4 G_SIMD(bit32x4 x, bit32x4 y, bit32x4 z) {
    return make_uint4(
        (x.x & z.x) | (y.x & ~z.x),
        (x.y & z.y) | (y.y & ~z.y),
        (x.z & z.z) | (y.z & ~z.z),
        (x.w & z.w) | (y.w & ~z.w));
}
__device__ __host__ inline bit32x4 H_SIMD(bit32x4 x, bit32x4 y, bit32x4 z) {
    return make_uint4(
        x.x ^ y.x ^ z.x,
        x.y ^ y.y ^ z.y,
        x.z ^ y.z ^ z.z,
        x.w ^ y.w ^ z.w);
}
__device__ __host__ inline bit32x4 I_SIMD(bit32x4 x, bit32x4 y, bit32x4 z) {
    return make_uint4(
        y.x ^ (x.x | ~z.x),
        y.y ^ (x.y | ~z.y),
        y.z ^ (x.z | ~z.z),
        y.w ^ (x.w | ~z.w));
}

// CUDA版本加法
__device__ __host__ inline bit32x4 vaddq_u32(bit32x4 a, bit32x4 b) {
    return make_uint4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
__device__ __host__ inline bit32x4 vdupq_n_u32(bit32 n) {
    return make_uint4(n, n, n, n);
}

// SIMD FF/GG/HH/II 宏
#define FF_SIMD(a, b, c, d, m, s, ti) { \
    bit32x4 _tmp = vaddq_u32(vaddq_u32(m, vdupq_n_u32(ti)), a); \
    _tmp = vaddq_u32(F_SIMD(b, c, d), _tmp); \
    a = vaddq_u32(b, ROTL_SIMD(_tmp, s)); \
}
#define GG_SIMD(a, b, c, d, m, s, ti) { \
    bit32x4 _tmp = vaddq_u32(vaddq_u32(m, vdupq_n_u32(ti)), a); \
    _tmp = vaddq_u32(G_SIMD(b, c, d), _tmp); \
    a = vaddq_u32(b, ROTL_SIMD(_tmp, s)); \
}
#define HH_SIMD(a, b, c, d, m, s, ti) { \
    bit32x4 _tmp = vaddq_u32(vaddq_u32(m, vdupq_n_u32(ti)), a); \
    _tmp = vaddq_u32(H_SIMD(b, c, d), _tmp); \
    a = vaddq_u32(b, ROTL_SIMD(_tmp, s)); \
}
#define II_SIMD(a, b, c, d, m, s, ti) { \
    bit32x4 _tmp = vaddq_u32(vaddq_u32(m, vdupq_n_u32(ti)), a); \
    _tmp = vaddq_u32(I_SIMD(b, c, d), _tmp); \
    a = vaddq_u32(b, ROTL_SIMD(_tmp, s)); \
}

// T和shifts外部声明
extern const bit32 T[64];
extern const int shifts[64];

// 标量MD5函数声明
void MD5Hash(std::string input, bit32 *state);
// SIMD MD5函数声明
void SIMD_MD5Hash(const std::string inputs[4], bit32 states[4][4]);