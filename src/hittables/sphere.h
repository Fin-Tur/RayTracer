#pragma once

#include "../rtweekend.h"
#include "hittable.h"
#include "../models/material.h"
#include "../models/bvh.h"

class sphere : public hittable {
    public:
    sphere(const point3& c, const double r, material* mat) : center(c), radius(std::fmax(r, 0)), mat(mat) {
    }
    ~sphere(){ delete(this->mat); }
    
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        point3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h -a*c;
        if (discriminant < 0) return false;

        auto sqrtd = std::sqrt(discriminant);
        auto root = (-h - sqrtd) / a;
        if(!ray_t.surrounds(root)){
            root = (-h + sqrtd) / a;
            if(!ray_t.surrounds(root)) return false;
        }

        rec.t = root;
        rec.p = r.ray_at(rec.t);
        vec3 outward_normal = (rec.p-center)/radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }

    bvh::aabb get_aabb() override{
        float min_x = center.x() - radius;
        float max_x = center.x() + radius;

        float min_y = center.y() - radius;
        float max_y = center.y() + radius;

        float min_z = center.z() - radius;
        float max_z = center.z() + radius;

        return {point3(min_x, min_y, min_z), point3(max_x, max_y, max_z)};
    }

    point3 get_center() const { return center; }
    double get_radius() const { return radius; }
    material* get_material() const {return mat; }

    private:
    point3 center;
    double radius;
    material* mat;
};