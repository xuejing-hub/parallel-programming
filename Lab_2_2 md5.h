#include <iostream>
#include <string>
#include <arm_neon.h>

using namespace std;

typedef uint32x4_t bit32x4;
typedef unsigned char Byte;
typedef unsigned int bit32;

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
/**
 * @Rotate Left.
 *
 * @param {num} the raw number.
 *
 * @param {n} rotate left n.
 *
 * @return the number after rotated left.
 */
// 定义了一系列MD5中的具体函数
// 这五个计算函数（ROTATELEFT/FF/GG/HH/II）和之前的FGHI一样，都是需要你进行SIMD并行化的
// 但是你需要注意的是#define的功能及其效果，可以发现这里的FGHI是没有返回值的，为什么呢？你可以查询#define的含义和用法


// SIMD版本的FGHI函数
// #define F_SIMD(x, y, z) vorrq_u32(vandq_u32(x, y), vandq_u32(vmvnq_u32(x), z))
// #define G_SIMD(x, y, z) vorrq_u32(vandq_u32(x, z), vandq_u32(y, vmvnq_u32(z)))
// #define H_SIMD(x, y, z) veorq_u32(veorq_u32(x, y), z)
// #define I_SIMD(x, y, z) veorq_u32(y, vorrq_u32(x, vmvnq_u32(z)))

// // SIMD版本的循环左移
// #define ROTATELEFT_SIMD(num, n) vorrq_u32(vshlq_n_u32(num, n), vshrq_n_u32(num, 32-(n)))
// // SIMD版本的轮函数
// #define FF_SIMD(a, b, c, d, x, s, ac) { \
//   bit32x4 temp = F_SIMD(b, c, d); \
//   temp = vaddq_u32(temp, x); \
//   temp = vaddq_u32(temp, vdupq_n_u32(ac)); \
//   a = vaddq_u32(a, temp); \
//   a = ROTATELEFT_SIMD(a, s); \
//   a = vaddq_u32(a, b); \
// }

// #define GG_SIMD(a, b, c, d, x, s, ac) { \
//   bit32x4 temp = G_SIMD(b, c, d); \
//   temp = vaddq_u32(temp, x); \
//   temp = vaddq_u32(temp, vdupq_n_u32(ac)); \
//   a = vaddq_u32(a, temp); \
//   a = ROTATELEFT_SIMD(a, s); \
//   a = vaddq_u32(a, b); \
// }

// #define HH_SIMD(a, b, c, d, x, s, ac) { \
//   bit32x4 temp = H_SIMD(b, c, d); \
//   temp = vaddq_u32(temp, x); \
//   temp = vaddq_u32(temp, vdupq_n_u32(ac)); \
//   a = vaddq_u32(a, temp); \
//   a = ROTATELEFT_SIMD(a, s); \
//   a = vaddq_u32(a, b); \
// }

// #define II_SIMD(a, b, c, d, x, s, ac) { \
//   bit32x4 temp = I_SIMD(b, c, d); \
//   temp = vaddq_u32(temp, x); \
//   temp = vaddq_u32(temp, vdupq_n_u32(ac)); \
//   a = vaddq_u32(a, temp); \
//   a = ROTATELEFT_SIMD(a, s); \
//   a = vaddq_u32(a, b); \
// }

// Left-rotate SIMD lanes by n bits
#define ROTL_SIMD(x, n) (vorrq_u32(vshlq_n_u32((x), (n)), vshrq_n_u32((x), (32 - (n)))))

// MD5 basic functions in SIMD
#define F_SIMD(x, y, z) vorrq_u32(vandq_u32((x), (y)), vandq_u32(vmvnq_u32((x)), (z)))
#define G_SIMD(x, y, z) vorrq_u32(vandq_u32((x), (z)), vandq_u32((y), vmvnq_u32((z))))
#define H_SIMD(x, y, z) veorq_u32(veorq_u32((x), (y)), (z))
#define I_SIMD(x, y, z) veorq_u32((y), vorrq_u32((x), vmvnq_u32((z))))

// Round macros
#define FF_SIMD(a, b, c, d, m, s, ti) \
    { \
        bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
        _tmp = vaddq_u32(F_SIMD((b), (c), (d)), _tmp); \
        (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
    }

#define GG_SIMD(a, b, c, d, m, s, ti) \
    { \
        bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
        _tmp = vaddq_u32(G_SIMD((b), (c), (d)), _tmp); \
        (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
    }

#define HH_SIMD(a, b, c, d, m, s, ti) \
    { \
        bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
        _tmp = vaddq_u32(H_SIMD((b), (c), (d)), _tmp); \
        (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
    }

#define II_SIMD(a, b, c, d, m, s, ti) \
    { \
        bit32x4 _tmp = vaddq_u32(vaddq_u32((m), vdupq_n_u32((ti))), (a)); \
        _tmp = vaddq_u32(I_SIMD((b), (c), (d)), _tmp); \
        (a) = vaddq_u32((b), ROTL_SIMD(_tmp, (s))); \
    }


// Extern declarations
extern const bit32 T[64];
extern const int shifts[64];



void MD5Hash(string input, bit32 *state);
void SIMD_MD5Hash(const string inputs[4], bit32 states[4][4]);
