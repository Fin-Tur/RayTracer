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
            gpu_converting::build_gpu_scene_large(&world, this->cam, scene);
        }else{
            gpu_converting::build_gpu_scene_small(&world, this->cam, scene);
        }
        gpu::launch_kernel(scene);
        gpu_converting::extract_framebuffer(scene, fb);
        gpu_converting::free_gpu_mem(scene);
    }

};