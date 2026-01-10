#pragma once

#if defined(__CUDACC__) || defined(__HIP_PLATFORM_AMD__)
#define HD __host__ __device__
#define H __host__
#define D __device__
#else
#define HD
#define D
#define H
#endif

#include "../rtweekend.h"
#include "../models/ray.h"
#include "../models/material.h"
#include "../hittables/hittable.h"
#include "../hittables/hittable_list.h"
#include "../hittables/sphere.h"
#include "../utils/camera.h"

#include <algorithm>
#include <iterator>
#include <map>

namespace gpu {

    D struct scene{
        sphere* spheres;
        uint32_t sphere_count;

        material* materials;
        uint32_t material_count;

        camera cam;

        color* framebuffer;
        uint32_t width{}, height{};
    };

    HD struct sphere{
        point3 center;
        float radius;
        uint32_t mat_id;
    };

    HD struct camera{ 
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

        D inline ray get_ray(int i, int j) const{
            auto offset = sample_square();
            auto pixel_sample = pixel00_loc + ((i+offset.x()) * pixel_delta_u) + ((j+offset.y()) * pixel_delta_v);

            auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
            auto ray_direction = pixel_sample - ray_origin;

            return ray(ray_origin, ray_direction);
        }

        D vec3 sample_square() const {
            return vec3(random_double() - 0.5, random_double() -0.5, 0);
        }

        D point3 defocus_disk_sample() const {
            auto p = random_in_unit_disk();
            return center + (p[0]*defocus_disk_u + p[1]*defocus_disk_v);
        }
    };

    HD enum material_type{
        metal,
        lambertian,
        dialetric

    };

    HD struct material {
        material_type type;
        color albedo;
        union{
            float fuzz;
            float ir;
        };

        
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

    
    D bool hit_sphere(const sphere& s, const ray& r, interval ray_t, hit_record& rec){
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

    
    D bool scatter_lambertian(const material& mat, const ray& ray_in, const hit_record& rec, ray& scattered, color& attentuation){
        vec3 direction = rec.normal + random_unit_vector();
        if(direction.near_zero()) direction = rec.normal;
        scattered = ray(rec.p, direction);
        attentuation = mat.albedo;
        return true;
    }

    D bool scatter_metal(const material& mat, const ray& r_in, const hit_record& rec, ray& scattered, color& attentuation){
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (mat.fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attentuation = mat.albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }

    D bool scatter_dialetric(const material& mat, const ray& r_in, const hit_record& rec, ray& scattered, color& attentuation){
        attentuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/mat.ir) : mat.ir;

        vec3 unit_direction = unit_vector(r_in.direction());

        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrtf(1.0 - cos_theta*cos_theta);

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

    
    D bool scatter(const material& mat, const ray& ray_in, const hit_record& rec, ray& scattered, color& attentuation){
        switch(mat.type){
            case lambertian:
                return scatter_lambertian(mat, ray_in, rec, scattered, attentuation);
            case dialetric:
                return scatter_dialetric(mat, ray_in, rec, scattered, attentuation);
            case metal:
                return scatter_metal(mat, ray_in, rec, scattered, attentuation);
        }
    }

    
    D color ray_color(ray& r, int depth, scene& scene){

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
        ->random_unit_vector (own RNG)
        ->HIP commands etc (HIPmemcpy, ...)
        ->Debug on CPU
        ->GPU Kernel
        ->BVH, Layout, divergence
    */


}

namespace gpu_converting {

    H gpu::material convert_material(material* mat){
        gpu::material gpu_mat;
        if(auto* m = dynamic_cast<const metal*>(mat)){
            gpu_mat.type = gpu::material_type::metal;
            gpu_mat.fuzz = m->get_fuzz();
            gpu_mat.albedo = m->get_albedo();
        }else if(auto* m = dynamic_cast<const lambertian*>(mat)){
            gpu_mat.type = gpu::material_type::lambertian;
            gpu_mat.albedo = m->get_albedo();
        }else if(auto* m = dynamic_cast<const dialectric*>(mat)){
            gpu_mat.type = gpu::material_type::dialetric;
            gpu_mat.ir = m->get_ri();
            gpu_mat.albedo = m->get_attentuation();
        }
        return gpu_mat;
    }

    H gpu::camera convert_camera(camera* cam){
        cam->initialize();
        return {cam->aspect_ratio,
                cam->image_width,
                cam->samples_per_pixel,
                cam->max_depth,
                cam->vfov,
                cam->lookfrom,
                cam->lookat,
                cam->vup,
                cam->defocus_angle,
                cam->focus_dist,
                cam->image_height,
                cam->pixel_samples_scale,
                cam->get_center(),
                cam->get_pixel00_loc(),
                cam->get_pixel_delta_u(),
                cam->get_pixel_delta_v(),
                cam->get_u(),
                cam->get_v(),
                cam->get_w(),
                cam->get_defocus_disk_u(),
                cam->get_defocus_disk_v()
                };
    }
    
    //Trade off lesser GPU Memory Bandwith to longer setup runtime through unique materials list and std::find
    //worth till ~250 mats
    H bool build_gpu_scene_small(hittable* world, camera* cam, gpu::scene& scene_out){
        if(typeid(*world) == typeid(hittable_list)){
            auto* list = static_cast<hittable_list*>(world);
            std::vector<gpu::sphere> spheres;
            spheres.reserve(list->objects.size());
            std::vector<gpu::material> materials;
            for(auto& obj : list->objects){
                if(auto* m = dynamic_cast<const sphere*>(obj)){
                    gpu::material mat = convert_material(m->get_material());
                    auto it = std::find(materials.begin(), materials.end(), mat);
                    if(it == materials.end()){
                        materials.emplace_back(mat);
                        spheres.emplace_back(m->get_center(), m->get_radius(), materials.size()-1);
                    }else{
                        spheres.push_back(gpu::sphere{m->get_center(), (float)m->get_radius(), (uint32_t)std::distance(materials.begin(), it)});
                    }
                }

            }
            scene_out.spheres = spheres.data();
            scene_out.materials = materials.data();
            scene_out.cam = convert_camera(cam);

            return true;
        }
        return false;
    }

    //Set version worth from ~250mats above
        H bool build_gpu_scene_large(hittable* world, camera* cam, gpu::scene& scene_out){
        if(typeid(*world) == typeid(hittable_list)){
            auto* list = static_cast<hittable_list*>(world);
            std::vector<gpu::sphere> spheres;
            spheres.reserve(list->objects.size());
            std::map<gpu::material, uint32_t> materials;
            uint16_t size;
            for(auto& obj : list->objects){
                if(auto* m = dynamic_cast<const sphere*>(obj)){
                    size = materials.size();
                    gpu::material gpu_m = convert_material(m->get_material());
                    auto [it, inserted] = materials.try_emplace(gpu_m, materials.size());
                    spheres.emplace_back(m->get_center(), m->get_radius(), it->second);
                }
            }
            std::vector<gpu::material> mats_vec;
            mats_vec.reserve(materials.size());

            for(auto& [mat, idx] : materials){
                mats_vec[idx] = mat;
            }

            scene_out.spheres = spheres.data();
            scene_out.materials = mats_vec.data();
            scene_out.cam = convert_camera(cam);
            //TODO -> mem
            return true;
        }
        return false;
    }

  }