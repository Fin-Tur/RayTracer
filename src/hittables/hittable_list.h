#pragma once

#include <vector>
#include "hittable.h"
#include "../rtweekend.h"


class hittable_list : public hittable {
    public:
    std::vector<std::shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(const std::shared_ptr<hittable> object) { add(object) ;}

    void clear() { objects.clear(); }
    void add(std::shared_ptr<hittable> obj) { objects.emplace_back(obj); }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override{
        hit_record temp_rec;
        bool hit_anything = false;
        auto clostest_so_far = ray_t.max;

        for(const auto& obj : objects){
            if(obj -> hit(r, interval(ray_t.min, clostest_so_far), temp_rec)){
                hit_anything = true;
                clostest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        return hit_anything;
    }

};