#pragma once
#include "vec3.h"

//using same class for vec3 and color since fun is the same, might lead to some misconceptions but who cares
using color = vec3;

//pixel_color vals [0, 1]
void write_color(std::ostream& out, const color& pixel_color){
    auto r = pixel_color[0];
    auto g = pixel_color[1];
    auto b = pixel_color[2];

    const double rbyte = (255.999 * r);
    const double gbyte = (255.999 * g);
    const double bbyte = (255.999 * b);

    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
    
}
