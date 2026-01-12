#pragma once

#if defined(__CUDACC__) || defined(__HIPCC__)
#include <hip/hip_runtime.h>
#define HD __host__ __device__
#define H __host__
#define D __device__
#else
#define HD
#define H
#define D
#endif

#include <stdint.h>


namespace rng{

    static constexpr double TWO_POW_32 = 4294967296.0;

    typedef struct{
        uint32_t num;
    }rng_state;

    inline HD void init_rng_state(rng_state& state, uint32_t tid){
        uint32_t x = tid;
        x = (x ^ 61u) ^ (x >> 16);
        x = x + (x << 3);
        x = x ^ (x >> 4);
        x = x * 0x27d4eb2du;
        x = x ^ (x >> 15);
        state.num = (x != 0) ? x : 0x6d2b79f5u; //fallback x = 0
    }

    //xorshift32
    inline HD uint32_t next_int(rng_state& prev_state){
        uint32_t& x = prev_state.num;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        prev_state.num = x;
        return x;
    }

    inline HD double next_double(rng_state& prev_state){
        double r = (double)next_int(prev_state);
        prev_state.num = r;
        return r * (1.0/TWO_POW_32);
    }

    inline HD double next_double_range(rng_state& prev_state, double min, double max){
        return min + (max-min) * next_double(prev_state);
    }

}