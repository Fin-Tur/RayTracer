#pragma once

#include "../rtweekend.h"
#include "hittable.h"

class sphere : public hittable {
    public:
    sphere(const point3& c, const double r) : center(c), radius(std::fmax(r, 0)) {}

    bool hit(const ray& r, double ray_tmin, double ray_tmax, hit_record& rec) const override {
        point3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - radius*radius;

        auto discriminant = h*h -a*c;
        if (discriminant < 0) return false;

        auto sqrtd = std::sqrt(discriminant);
        auto root = (-h - sqrtd) / a;
        if(root < ray_tmin || root > ray_tmax){
            root = (-h + sqrtd) / a;
            if(root < ray_tmin || root > ray_tmax) return false;
        }

        rec.t = root;
        rec.p = r.ray_at(rec.t);
        vec3 outward_normal = (rec.p-center)/radius;
        rec.set_face_normal(r, outward_normal);

        return true;
    }

    private:
    point3 center;
    double radius;
};