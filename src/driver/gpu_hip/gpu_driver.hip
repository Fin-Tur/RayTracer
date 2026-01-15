
#include <hip/hip_runtime.h>
#include "gpu_driver.h"

#include "../../models/material.h"      
#include "../../hittables/sphere.h"      
#include "../../utils/camera.h"          
#include "../../hittables/hittable_list.h"
#include "../../models/tile.h"
#include "../renderer.h"

namespace gpu{

    __global__ void render_kernel(gpu::scene* scene, int tile_offset_x, int tile_offset_y){
        int x = blockIdx.x * blockDim.x + threadIdx.x + tile_offset_x;
        int y = blockIdx.y * blockDim.y + threadIdx.y + tile_offset_y;
        
        if(x >= scene->width || y >= scene->height) return;
        
        uint32_t pixel_idx = y * scene->width + x;
        rng::rng_state rng_state{};
        rng::init_rng_state(rng_state, pixel_idx);
        
        color cl(0, 0, 0);
        for(int i = 0; i < scene->cam.samples_per_pixel; ++i){
            ray r = scene->cam.get_ray(x, y, rng_state);
            cl += ray_color(r, scene->cam.max_depth, *scene, rng_state);
        }
        scene->framebuffer[pixel_idx] = cl / scene->cam.samples_per_pixel;
    }

    H void launch_kernel(gpu::scene& scene, tiles::tile_ctx t_ctx){
        gpu::scene* d_scene;
        assert(hipMalloc(&d_scene, sizeof(gpu::scene)) == hipSuccess);
        assert(hipMemcpy(d_scene, &scene, sizeof(gpu::scene), hipMemcpyHostToDevice) == hipSuccess);
        
        int total_tiles = t_ctx.tiles_in_column * t_ctx.tiles_in_row;
        int current_tile = 0;

        dim3 blockSize(16, 16);
        dim3 gridSize((t_ctx.size_sqrt + 15) / 16, 
            (t_ctx.size_sqrt + 15) / 16);

        for(int ty = 0; ty < t_ctx.tiles_in_column; ty++){
            for(int tx = 0; tx < t_ctx.tiles_in_row; tx++){
                int offset_x = tx*t_ctx.size_sqrt;
                int offset_y = ty*t_ctx.size_sqrt;      
                hipLaunchKernelGGL(render_kernel, gridSize, blockSize, 0, 0, d_scene, offset_x, offset_y);
                current_tile++;
                assert(hipDeviceSynchronize() == hipSuccess);
                renderer::print_progress(int((float(current_tile)/total_tiles)*100));
            }
        }
        assert(hipFree(d_scene) == hipSuccess);
        std::clog << RESET << "\n" << std::flush;
    }

namespace converting{

    void H free_gpu_mem(gpu::scene& scene){
        assert(hipFree(scene.framebuffer) == hipSuccess);
        assert(hipFree(scene.materials) == hipSuccess);
        assert(hipFree(scene.spheres) == hipSuccess);
    }

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

    H gpu::camera inline create_pod_cam(::camera* cam, gpu::scene& scene_out){
        //Cam initialization done in cli/reader
        scene_out.height = cam->image_height;
        scene_out.width = cam->image_width;
        assert(hipMalloc(&scene_out.framebuffer, scene_out.height*scene_out.width*sizeof(color)) == hipSuccess);
        
        // Allocate and initialize camera on device
        gpu::camera gpu_cam = {cam->aspect_ratio,
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
        return gpu_cam;
    }
    
    //Trade off lesser GPU Memory Bandwith to longer setup runtime through unique materials list and std::find
    //worth till ~50 mats
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
            
            create_pod_cam(cam, scene_out);
            assert(hipMalloc(&scene_out.spheres, spheres.size()*sizeof(gpu::sphere))== hipSuccess);
            assert(hipMalloc(&scene_out.materials, materials.size()*sizeof(gpu::material)) == hipSuccess);
            assert(hipMemcpy(scene_out.spheres, spheres.data(), spheres.size()*sizeof(gpu::sphere), hipMemcpyHostToDevice) == hipSuccess);
            assert(hipMemcpy(scene_out.materials, materials.data(), materials.size()*sizeof(gpu::material), hipMemcpyHostToDevice) == hipSuccess);
            scene_out.sphere_count = spheres.size();
            scene_out.material_count = materials.size();

            return true;
        }
        return false;
    }

    //Set version worth from ~50mats above
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
            mats_vec.resize(materials.size());

            for(auto& [mat, idx] : materials){
                mats_vec[idx] = mat;
            }  

            scene_out.cam = create_pod_cam(cam, scene_out);
            assert(hipMalloc(&scene_out.spheres, spheres.size()*sizeof(gpu::sphere)) == hipSuccess);
            assert(hipMalloc(&scene_out.materials, mats_vec.size()*sizeof(gpu::material)) == hipSuccess);
            assert(hipMemcpy(scene_out.spheres, spheres.data(), spheres.size()*sizeof(gpu::sphere), hipMemcpyHostToDevice) == hipSuccess);
            assert(hipMemcpy(scene_out.materials, mats_vec.data(), mats_vec.size()*sizeof(gpu::material), hipMemcpyHostToDevice) == hipSuccess);
            scene_out.sphere_count = spheres.size();
            scene_out.material_count = mats_vec.size();

            return true;
            }
        return false;
        }

        H bool extract_framebuffer(gpu::scene& scene, color* fb){
            return hipMemcpy(fb, scene.framebuffer, scene.height*scene.width*sizeof(color), hipMemcpyDeviceToHost);
        }

    }
}