void SIMD_MD5Hash(const std::string inputs[2], bit32 states[4][4]) {
    // 初始化MD5向量，每个向量保存4个并行的状态
    bit32x4 state[4] = {
        vdupq_n_u32(0x67452301),
        vdupq_n_u32(0xefcdab89),
        vdupq_n_u32(0x98badcfe),
        vdupq_n_u32(0x10325476)
    };

    // 为2个输入分别进行预处理
    Byte* paddedMessages[2];
    int messageLengths[2];
    int maxBlocks = 0;

    // 并行预处理2个输入
    for(int i = 0; i < 2; i++) {
        paddedMessages[i] = StringProcess(inputs[i], &messageLengths[i]);
        int blocks = messageLengths[i] / 64;
        maxBlocks = max(maxBlocks, blocks);
    }

    // 处理所有块
    for(int block = 0; block < maxBlocks; block++) {
        bit32x4 x[16];  // 用于存储2个输入的当前块数据
        
        // 并行加载2个输入的块数据
        for(int j = 0; j < 16; j++) {
            uint32_t values[2] = {0, 0};  // 只需要两个值
            for(int i = 0; i < 2; i++) {
                if(block * 64 < messageLengths[i]) {
                    int offset = block * 64 + j * 4;
                    values[i] = ((uint32_t)paddedMessages[i][offset]) |
                               ((uint32_t)paddedMessages[i][offset + 1] << 8) |
                               ((uint32_t)paddedMessages[i][offset + 2] << 16) |
                               ((uint32_t)paddedMessages[i][offset + 3] << 24);
                }
            }
            // 一次加载2个值到SIMD寄存器
            x[j] = vld1q_u32(values);
        }

        // 保存当前状态
        bit32x4 a = state[0];
        bit32x4 b = state[1];
        bit32x4 c = state[2];
        bit32x4 d = state[3];

        // MD5压缩函数的4轮操作
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
        state[0] = vaddq_u32(state[0], a);
        state[1] = vaddq_u32(state[1], b);
        state[2] = vaddq_u32(state[2], c);
        state[3] = vaddq_u32(state[3], d);
    }

    // 保存并调整字节序
    for(int i = 0; i < 4; i++) {
        states[0][i] = vgetq_lane_u32(state[i], 0);
        states[1][i] = vgetq_lane_u32(state[i], 1);
        states[2][i] = vgetq_lane_u32(state[i], 2);
        states[3][i] = vgetq_lane_u32(state[i], 3);

        // 调整每个结果的字节序
        for(int j = 0; j < 4; j++) {
            uint32_t value = states[j][i];
            states[j][i] = ((value & 0xff) << 24) |
                          ((value & 0xff00) << 8) |
                          ((value & 0xff0000) >> 8) |
                          ((value & 0xff000000) >> 24);
        }
    }

    // 清理
    for(int i = 0; i < 2; i++) {
        delete[] paddedMessages[i];
    }
}
