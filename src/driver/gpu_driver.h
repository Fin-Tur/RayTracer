#pragma once
#include "../rtweekend.h"
#include "../models/ray.h"
#include "../hittables/hittable.h"

namespace gpu {

    struct scene{
        sphere* spheres;
        uint32_t sphere_count;

        material* materials;
        uint32_t material_count;
    };

    struct sphere{
        point3 center;
        float radius;
        uint32_t mat_id;
    };

    enum material_type{
        metal,
        lambertian,
        dialetric
    };

    struct material {
        material_type type;
        color albedo;
        union{
            float fuzz;
            float ir;
        };
    };

    struct hit_record {
        point3 p;
        vec3 normal;
        float t;
        bool front_face;
        uint32_t mat_id;  

        inline void set_face_normal(const ray& r, const vec3& outward_normal, hit_record& rec){
        rec.front_face = dot(r.direction(), outward_normal) < 0;
        rec.normal = rec.front_face ? outward_normal : -outward_normal;
        
    }

    };

    bool hit_sphere(const sphere& s, const ray& r, interval ray_t, hit_record& rec){
        point3 oc = r.origin() - s.center;
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - s.radius*s.radius;

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
        vec3 outward_normal = (rec.p-s.center)/s.radius;
        rec.set_face_normal(r, outward_normal, rec);
        rec.mat_id = s.mat_id;

        return true;
    }

    bool hit_world(scene& scene, ray& ray, interval ray_t, hit_record& rec){
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;
        hit_record temp_rec;

        for(int i = 0; i < scene.sphere_count; i++){
            if(hit_sphere(scene.spheres[i], ray, interval(ray_t.min, closest_so_far), temp_rec)){
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }
        return hit_anything;
    }


}