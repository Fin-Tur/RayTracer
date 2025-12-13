#pragma once

#include "vec3.h"
#include "interval.h"

//using same class for vec3 and color since fun is the same, might lead to some misconceptions but who cares
using color = vec3;

inline double linear_to_gamma(double linear_component){
    if(linear_component > 0) return std::sqrt(linear_component);
    return 0;
}

//pixel_color vals [0, 1]
void write_color(std::ostream& out, const color& pixel_color){
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    static const interval intensity(0.000, 0.999);
    int rbyte = (256 * intensity.clamp(r));
    int gbyte = (256 * intensity.clamp(g));
    int bbyte = (256 * intensity.clamp(b));
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
    
}
