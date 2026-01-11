
#include <hip/hip_runtime.h>
#include "gpu_driver.h"

#include "../../models/material.h"      
#include "../../hittables/sphere.h"      
#include "../../utils/camera.h"          
#include "../../hittables/hittable_list.h"

namespace gpu{
    namespace converting{
        H gpu::material convert_material(::material* mat){
        gpu::material gpu_mat;
        if(auto* m = dynamic_cast<const ::metal*>(mat)){
            gpu_mat.type = gpu::material_type::metal;
            gpu_mat.fuzz = m->get_fuzz();
            gpu_mat.albedo = m->get_albedo();
        }else if(auto* m = dynamic_cast<const ::lambertian*>(mat)){
            gpu_mat.type = gpu::material_type::lambertian;
            gpu_mat.albedo = m->get_albedo();
        }else if(auto* m = dynamic_cast<const ::dialectric*>(mat)){
            gpu_mat.type = gpu::material_type::dialetric;
            gpu_mat.ir = m->get_ri();
            gpu_mat.albedo = m->get_attentuation();
        }
        return gpu_mat;
    }

    H gpu::camera convert_camera(::camera* cam){
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
    H bool build_gpu_scene_small(::hittable* world, ::camera* cam, gpu::scene& scene_out){
        if(typeid(*world) == typeid(::hittable_list)){
            auto* list = static_cast<::hittable_list*>(world);
            std::vector<gpu::sphere> spheres;
            spheres.reserve(list->objects.size());
            std::vector<gpu::material> materials;
            for(auto& obj : list->objects){
                if(auto* m = dynamic_cast<const ::sphere*>(obj)){
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
        H bool build_gpu_scene_large(::hittable* world, ::camera* cam, gpu::scene& scene_out){
        if(typeid(*world) == typeid(::hittable_list)){
            auto* list = static_cast<::hittable_list*>(world);
            std::vector<gpu::sphere> spheres;
            spheres.reserve(list->objects.size());
            std::map<gpu::material, uint32_t> materials;
            uint16_t size;
            for(auto& obj : list->objects){
                if(auto* m = dynamic_cast<const ::sphere*>(obj)){
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
        }
    }
}