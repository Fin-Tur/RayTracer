#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>

//Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

//Utility funcs
inline double degrees_to_radians(const double deg) {
    return deg * pi / 180.0;
}

//Common Headers:

#include "models/color.h"
#include "models/vec3.h"
#include "models/ray.h"
#include "models/interval.h"