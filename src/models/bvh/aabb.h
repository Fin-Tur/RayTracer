#pragma once

#include "../../rtweekend.h"
#include "../ray.h"

struct aabb {
        point3 min;
        point3 max;

        inline void include(aabb&& other){
            min = vec_min(min, other.min); 
            max = vec_max(max, other.max);
        } 

        inline bool hit_box(const ray& r, float tmin, float tmax){
            for(int a = 0; a < 3; ++a){
                const double invD = 1.0 / r.direction()[a];
                double t0 = (min[a] - r.origin()[a]) * invD;
                double t1 = (max[a] - r.origin()[a]) * invD;
                if(invD < 0.0 ) std::swap(t0, t1);

                tmin = fmax(tmin, t0);
                tmax = fmin(tmax, t1);

                if (tmax <= tmin) return false;

            }

            return true;
        }

    };