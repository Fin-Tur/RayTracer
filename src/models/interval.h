#pragma once

#if defined(__CUDACC__) || defined(__HIP_PLATFORM_AMD__)
#define HD __host__ __device__
#define H __host__
#define D __device__
#else
#define HD
#define H
#define D
#endif

#include "../rtweekend.h"

class interval
{
public:
    double min, max;

    HD interval() : min(+infinity), max(-infinity) {}
    HD interval(double minimum, double maximum) : min(minimum), max(maximum) {}

    double size() const
    {
        return max - min;
    }

    bool contains(double val) const
    {
        return min <= val && val <= max;
    }

    HD bool surrounds(double val) const
    {
        return min < val && val < max;
    }

    double clamp(double x) const {
        if (min > x) {
            return min;
        }
        if(max < x){
            return max;
        }
        return x;
    }

    static const interval empty, universe;
};

inline const interval interval::empty = interval();
inline const interval interval::universe = interval(-infinity, +infinity);