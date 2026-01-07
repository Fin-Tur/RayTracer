#pragma once

#ifdef __CUDACC__
#define HD = __host__ __device__
#endif

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

        inline double reflectance(double cosine, double refraction_index) const{
        //Schlicks approx
        auto r0 = (1-refraction_index) / (1+refraction_index);
        r0 *= r0;
        return r0 + (1-r0)*std::pow((1-cosine),5);
    }
    };

    struct hit_record {
        point3 p;
        vec3 normal;
        float t;
        bool front_face;
        uint32_t mat_id;  

        inline void set_face_normal(const ray& r, const vec3& outward_normal){
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
        
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
        rec.set_face_normal(r, outward_normal);
        rec.mat_id = s.mat_id;

        return true;
    }

    bool hit_world(const scene& scene, const ray& ray, interval ray_t, hit_record& rec){
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

    bool scatter(const material& mat, const ray& ray_in, const hit_record& rec, ray& scattered, color& attentuation){
        switch(mat.type){
            case lambertian:
                return scatter_lambertian(mat, ray_in, rec, scattered, attentuation);
            case dialetric:
                return scatter_dialetric(mat, ray_in, rec, scattered, attentuation);
            case metal:
                return scatter_metal(mat, ray_in, rec, scattered, attentuation);
        }
    }

    bool scatter_lambertian(const material& mat, const ray& ray_in, const hit_record& rec, ray& scattered, color& attentuation){
        vec3 direction = rec.normal + random_unit_vector();
        if(direction.near_zero()) direction = rec.normal;
        scattered = ray(rec.p, direction);
        attentuation = mat.albedo;
        return true;
    }

    bool scatter_metal(const material& mat, const ray& r_in, const hit_record& rec, ray& scattered, color& attentuation){
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (mat.fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attentuation = mat.albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }

    bool scatter_dialetric(const material& mat, const ray& r_in, const hit_record& rec, ray& scattered, color& attentuation){
        attentuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/mat.ir) : mat.ir;

        vec3 unit_direction = unit_vector(r_in.direction());

        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if(cannot_refract || mat.reflectance(cos_theta, ri) > random_double()){
            direction = reflect(unit_direction, rec.normal);
        }else{
            direction = refract(unit_direction, rec.normal, ri);
        }

        scattered = ray(rec.p, direction);
        return true;
    }

    color ray_color(ray& r, int depth, scene& scene){

        color accumulated = color(1.0, 1.0, 1.0);  //Maybe
        
        while(depth > 0){
            hit_record rec;

            if (!(hit_world(scene, r, interval(0.001, infinity), rec))){
                vec3 unit_direction = unit_vector(r.direction());
                auto a = 0.5*(unit_direction.y() + 1.0);
                return accumulated * (1.0-a) * color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);

            }
            ray scattered;
            color attentuation;
            if(scatter(scene.materials[rec.mat_id], r, rec, scattered, attentuation)){
                accumulated *= attentuation;
                depth--;
                r = scattered;
            }else{
                break;
            }
        }
        return color(0,0,0);
  }

    /*
    ->NEXT
        ->converting methods 
        ->random_unit_vector (own RNG)
        ->Debug on CPU
        ->GPU Kernel

        ->BVH, Layout, divergence
    */


}