#pragma once

#if defined(__CUDACC__) || defined(__HIP_PLATFORM_AMD__)
#define HD __host__ __device__
#define H __host__
#define D __device__
#include <hip/hip_runtime.h>
#else
#define HD
#define H
#define D
#endif

#include "../../rtweekend.h"
#include "../../models/ray.h"
#include "../../models/material.h"
#include "../../hittables/hittable.h"
#include "../../hittables/hittable_list.h"
#include "../../hittables/sphere.h"
#include "../../utils/camera.h"
#include "rng__seeded.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <typeinfo>

namespace gpu {

    H void print_test();

    struct sphere;
    struct material;
    struct camera;

    struct scene{
        sphere* spheres;
        uint32_t sphere_count;

        material* materials;
        uint32_t material_count;

        camera* cam;  // Pointer statt Wert, da camera noch nicht vollständig definiert ist

        color* framebuffer;
        uint32_t width{}, height{};
    };

    struct sphere{
        point3 center;
        float radius;
        uint32_t mat_id;

        sphere(vec3 c, double r, unsigned m)
        : center(c), radius(r), mat_id(m) {}
    };

    D vec3 random_unit_vector(rng::rng_state& rng) {
        while(true){
            auto p = vec3::random_rng_range(rng,-1,1);
            auto lensq = p.length_squared();
            if(lensq > 1e-160 && lensq <= 1){
                return p / sqrtf(lensq);
            }
        }
    }

    D vec3 random_in_unit_disk(rng::rng_state& rng){
            while (true){
                auto p = vec3(rng::next_double_range(rng, -1, 1), rng::next_double_range(rng, -1, 1), 0);
                if (p.length_squared() < 1){
                    return p;
                }
            }
        }

    D vec3 sample_square(rng::rng_state& rng){
        return vec3(rng::next_double(rng) - 0.5, rng::next_double(rng) -0.5, 0);
    }

    struct camera{ 
        double aspect_ratio = 1.0;
        int image_width = 100; 
        int samples_per_pixel = 10; 
        int max_depth = 10; 
        double vfov;
        point3 lookfrom = point3(0,0,0); 
        point3 lookat = point3(0,0,-1); 
        vec3 vup = vec3(0,1,0); 
        double defocus_angle = 0; 
        double focus_dist = 10; 
        int image_height; 
        double pixel_samples_scale;
        point3 center;
        point3 pixel00_loc;
        vec3 pixel_delta_u;
        vec3 pixel_delta_v;
        vec3 u, v, w;
        vec3 defocus_disk_u;
        vec3 defocus_disk_v;

        D inline ray get_ray(int i, int j, rng::rng_state& rng) const{
            auto offset = sample_square(rng);
            auto pixel_sample = pixel00_loc + ((i+offset.x()) * pixel_delta_u) + ((j+offset.y()) * pixel_delta_v);

            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample(rng);
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
        }

        D point3 defocus_disk_sample(rng::rng_state& rng) const {
            auto p = random_in_unit_disk(rng);
            return center + (p[0]*defocus_disk_u + p[1]*defocus_disk_v);
        }
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

        // Comparison operator für std::map
        HD bool operator<(const material& other) const {
            if(type != other.type) return type < other.type;
            if(!(albedo == other.albedo)) {
                if(albedo.x() != other.albedo.x()) return albedo.x() < other.albedo.x();
                if(albedo.y() != other.albedo.y()) return albedo.y() < other.albedo.y();
                return albedo.z() < other.albedo.z();
            }
            // Compare union basierend auf type
            if(type == material_type::metal) return fuzz < other.fuzz;
            if(type == material_type::dialetric) return ir < other.ir;
            return false;
        }
        
        D inline double reflectance(double cosine, double refraction_index) const{
            //Schlicks approx
            auto r0 = (1-refraction_index) / (1+refraction_index);
            r0 *= r0;
            return r0 + (1-r0)*powf((1-cosine),5);
        }

        H bool operator==(const material& other) const{
            return (other.type == type && other.albedo == albedo && (other.fuzz == fuzz || other.ir == ir));
        }

    };

    struct hit_record {
        point3 p;
        vec3 normal;
        float t;
        bool front_face;
        uint32_t mat_id;  

        
        D inline void set_face_normal(const ray& r, const vec3& outward_normal){
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
        
    }

    };
    
    D bool hit_sphere(const gpu::sphere& s, const ray& r, interval ray_t, hit_record& rec){
        point3 oc = r.origin() - s.center;
        auto a = r.direction().length_squared();
        auto h = dot(r.direction(), oc);
        auto c = oc.length_squared() - s.radius*s.radius;

        auto discriminant = h*h -a*c;
        if (discriminant < 0) return false;

        auto sqrtd = sqrtf(discriminant);
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

    
    D bool hit_world(const scene& scene, const ray& ray, interval ray_t, hit_record& rec){
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

    
    D bool scatter_lambertian(const material& mat, const ray& ray_in, const hit_record& rec, ray& scattered, color& attentuation, rng::rng_state& rng){
        vec3 direction = rec.normal + random_unit_vector(rng);
        if(direction.near_zero()) direction = rec.normal;
        scattered = ray(rec.p, direction);
        attentuation = mat.albedo;
        return true;
    }

    D bool scatter_metal(const material& mat, const ray& r_in, const hit_record& rec, ray& scattered, color& attentuation, rng::rng_state& rng){
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (mat.fuzz * random_unit_vector(rng));
        scattered = ray(rec.p, reflected);
        attentuation = mat.albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }

    D bool scatter_dialetric(const material& mat, const ray& r_in, const hit_record& rec, ray& scattered, color& attentuation, rng::rng_state& rng){
        attentuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/mat.ir) : mat.ir;

        vec3 unit_direction = unit_vector(r_in.direction());

        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrtf(1.0 - cos_theta*cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        vec3 direction;

        if(cannot_refract || mat.reflectance(cos_theta, ri) > rng::next_double(rng)){
            direction = reflect(unit_direction, rec.normal);
        }else{
            direction = refract(unit_direction, rec.normal, ri);
        }

        scattered = ray(rec.p, direction);
        return true;
    }

    
    D bool scatter(const material& mat, const ray& ray_in, const hit_record& rec, ray& scattered, color& attentuation, rng::rng_state& rng){
        switch(mat.type){
            case lambertian:
                return scatter_lambertian(mat, ray_in, rec, scattered, attentuation, rng);
            case dialetric:
                return scatter_dialetric(mat, ray_in, rec, scattered, attentuation, rng);
            case metal:
                return scatter_metal(mat, ray_in, rec, scattered, attentuation, rng);
        }
        return false;
    }

    
    D color ray_color(ray& r, int depth, scene& scene, rng::rng_state& rng){

        color accumulated = color(1.0, 1.0, 1.0);  //Maybe
        
        while(depth > 0){
            hit_record rec;

            if (!(hit_world(scene, r, interval(0.001, infinity), rec))){
                vec3 unit_direction = unit_vector(r.direction());
                auto a = 0.5*(unit_direction.y() + 1.0);
                return accumulated * ((1.0-a) * color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0));

            }
            ray scattered;
            color attentuation;
            if(scatter(scene.materials[rec.mat_id], r, rec, scattered, attentuation, rng)){
                accumulated *= attentuation;
                depth--;
                r = scattered;
            }else{
            return color(0,0,0);
            }
        }
        vec3 unit_direction = unit_vector(r.direction());
        auto a = 0.5*(unit_direction.y() + 1.0);
        return accumulated * ((1.0-a) * color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0));
    }


#if defined(__CUDACC__) || defined(__HIP_PLATFORM_AMD__)
  __global__ void render_kernel(gpu::scene* scene);
#endif

  //Entry point
  H void launch_kernel(gpu::scene& scene);

    /*
    ->NEXT
        ->Debug on CPU
        ->GPU Kernel
        ->BVH, Layout, divergence
    */

}

namespace gpu_converting {

        H void free_gpu_mem(gpu::scene& scene);

        H gpu::material convert_material(material* mat);

        H void create_pod_cam(camera* cam, gpu::scene& scene_out);
        
        H bool build_gpu_scene_small(hittable* world, camera* cam, gpu::scene& scene_out);

        H bool build_gpu_scene_large(hittable* world, camera* cam, gpu::scene& scene_out);

        H bool extract_framebuffer(gpu::scene& scene, color* fb); 

    }

