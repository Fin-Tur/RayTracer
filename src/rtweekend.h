#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <random>

// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// Utility funcs
inline double degrees_to_radians(const double deg)
{
    return deg * pi / 180.0;
}

inline double random_double()
{
    static std::uniform_real_distribution<double> distritubtion(0.0, 1.0);
    static std::mt19937 gen;
    return distritubtion(gen);
}

inline double random_double(double min, double max)
{
    return min + (max - min) * random_double();
}

// Common Headers:
#include "models/color.h"
#include "models/vec3.h"
#include "models/ray.h"
#include "models/interval.h"