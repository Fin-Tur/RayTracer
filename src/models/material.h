#pragma once

#include "../hittables/hittable.h"
#include "../rtweekend.h"

class material {
    public:
    virtual ~material() = default;
    virtual bool scatter (const ray& r_in, const hit_record&, color& attentuation, ray& scattered) {
        
    }
    
};