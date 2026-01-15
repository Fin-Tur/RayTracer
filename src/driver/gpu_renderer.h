#pragma once

#include "gpu_hip/gpu_driver.h"
#include "renderer.h"

class gpu_renderer : public renderer{

    public:

    gpu_renderer(camera* cam) : renderer(cam) {}

    void start_rendering(hittable& world) override {
        color* fb = frame_buffer.data();
        gpu::scene scene;
        auto* world_converted = dynamic_cast<hittable_list*>(&world);
        if(world_converted->objects.size() > 50){
            gpu::converting::build_gpu_scene_large(&world, this->cam, scene);
        }else{
            gpu::converting::build_gpu_scene_small(&world, this->cam, scene);
        }
        tiles::tile_ctx t_ctx;
        tiles::calculate_tile_placement(t_ctx, this->cam, 64);
        std::clog << "Launching GPU Kernel with tile size " << t_ctx.size_sqrt << ": \n";
        gpu::launch_kernel(scene, t_ctx);
        gpu::converting::extract_framebuffer(scene, fb);
        gpu::converting::free_gpu_mem(scene);
    }

};