#pragma once
#include "renderer.h"


class base_renderer : public renderer{

    public:

    base_renderer(const std::shared_ptr<camera> camera): renderer(camera) {}

    void start_rendering(const hittable& world) override{
        auto* fb = frame_buffer.data();
        for(size_t j = 0; j < cam->image_height; j++){
            for(size_t i = 0; i < cam->image_width; i++){
                if(j%50 == 0 || i%50 == 0){
                    std::clog << "\rRendering: " << (int)((float)cam->image_height*cam->image_width/(j*cam->image_width + i)) << "% " << std::flush;
                }
                color pixel_color(0,0,0);
                for(int sample = 0; sample < cam->samples_per_pixel; sample++){
                    auto curr_ray = cam->get_ray(i, j);
                    pixel_color += cam->ray_color(curr_ray, cam->max_depth ,world);
                }  
                fb[j*cam->image_width + i] = pixel_color;
            }
        }
    }

};