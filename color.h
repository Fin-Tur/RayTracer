#pragma once
#include "vec3.h"

//using same class for vec3 and color since fun is the same, might lead to some misconceptions but who cares
using color = vec3;

//pixel_color vals [0, 1]
void write_color(std::ostream& out, const color& pixel_color){
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    const int rbyte = static_cast<int>(255.999 * r);
    const int gbyte = static_cast<int>(255.999 * g);
    const int bbyte = static_cast<int>(255.999 * b);

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
    
}
