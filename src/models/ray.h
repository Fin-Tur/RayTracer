#pragma once

#if defined(__CUDACC__) || defined(__HIP_PLATFORM_AMD__)
#define HD __host__ __device__
#define H __host__ 
#define D __device__
#include <hip/hip_runtime.h>
#else
#define HD
#define H
#define D
#endif

#include "vec3.h"


class ray {

    public:
    HD ray() {}
    HD ray(const point3& origin, const vec3& direction, double time) : orig(origin), dir(direction), tm(time) {}
    HD ray(const point3& origin, const vec3& direction) : orig(origin), dir(direction), tm(0.0) {}

    HD const point3& origin() const{ return orig; }
    HD const vec3& direction() const{ return dir; }

    double time() const {return tm;}

    HD point3 ray_at(const double t) const{
        return orig + t*dir;
    } 

    private:
    point3 orig;
    vec3 dir;
    double tm;
};