#pragma once

#include "../rtweekend.h"

class interval {
    public: 
    double min, max;

    interval() : min(+infinity), max(-infinity) {}
    interval(double minimum, double maximum) : min(minimum), max(maximum) {}

    double size() const {
        return max-min;
    }

    bool contains(double val) const {
        return min <= val && val <= max;
    }

    bool surrounds(double val) const {
        return min < val && val < max;
    }

    static const interval empty, universe;

};

const interval interval::empty = interval();
const interval interval::universe = interval(-infinity, +infinity);