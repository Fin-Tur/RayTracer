#pragma once
#include "../utils/camera.h"
#include "../rtweekend.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <thread>

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

    renderer(const camera* const camera) : cam(camera) {
        frame_buffer.resize(cam->image_height*cam->image_width);
    }

    void print_progress(int progress){
        std::ostringstream ss;
        std::string color;
        ss << "\rRendering :" << std::setw(3) << progress << "% ";
        if(progress < 40){
            color = RED;
        }else if(progress < 80){
            color = YELLOW;
        }else{
            color = GREEN;
        }
        ss << color << "|" 
        << std::string(progress/10, '=') << std::string(10-progress/10, ' ') << "|";
        std::clog << ss.str() << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    const camera* const cam;
    std::vector<color> frame_buffer;

};