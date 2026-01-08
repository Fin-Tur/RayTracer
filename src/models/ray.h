#pragma once

#ifdef __CUDACC__
#define HD __host__ __device__
#else
#define HD
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