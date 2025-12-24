#pragma once
#include "../utils/camera.h"

class renderer {

    public:

    virtual void start_rendering(const hittable& world) = 0;

    void print_rgbs(std::ostream& out) {
        auto* fb = frame_buffer.data();
        out << "P3\n"<< cam->image_width << " " << cam->image_height << "\n255\n";
        for (size_t j = 0; j < cam->image_height; j++){
            for (size_t i = 0; i < cam->image_width; i++){
                auto pixel = j*cam->image_width + i;
                color& c = fb[pixel];
                write_color(out, c);
            }
        }
    }

    void empty_frame_buffer(){
        frame_buffer.clear();
    }

    protected:
    renderer(const std::shared_ptr<camera>& camera) : cam(camera) {
        frame_buffer.resize(cam->image_height*cam->image_width);
    }
    
    const std::shared_ptr<camera> cam;
    std::vector<color> frame_buffer;

};