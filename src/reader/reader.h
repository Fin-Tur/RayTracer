#pragma once

#include "../hittables/sphere.h"
#include "../utils/camera.h"
#include "../api/config.h"
#include "../driver/multithreading.h"
#include "../driver/base_renderer.h"

#include <regex>
#include <filesystem>
#include <optional>
#include <stdio.h>
#include <fstream>
#include <algorithm>

namespace reader {

    namespace intern{

        enum form {
            f_sphere
        };

        enum mat {
            m_metal,
            m_dialetric,
            m_lambertian
        };

        struct obj {
            form obj_form;
            mat obj_mat;
            point3 cords;
            double size;
            color rgb;
            union {
                double fuzz;
                double dialetric;
            };
        };

        static const std::regex universal_pattern(R"((\S+)\s+\(([^)]+)\)\s+(\S+)\s+(\S+)\s+(.*))");

        inline void add_obj_to_hit_list(hittable_list* h_list, obj o){
            material* material;
            switch(o.obj_mat){
                case m_metal:
                    material = new metal(o.rgb, o.fuzz);
                    break;
                case m_lambertian:
                    material = new lambertian(o.rgb);
                    break;
                case m_dialetric:
                    material = new dialectric(o.dialetric);
                    break;
            }

            h_list->add(new sphere(o.cords, o.size, material));
        }

        inline hittable* read_scene(std::filesystem::path src){
            if(!std::filesystem::is_regular_file(src) || src.extension() != ".tsc"){
                return nullptr;
            } 
            try{
                hittable_list* hit_list = new hittable_list;
                std::ifstream f(src);
                if(!f.is_open()){
                    return nullptr;
                }
                std::string line;
                std::smatch matches; //Sphere, Coords, size, material, attributes
                while(std::getline(f, line)){
                    if(line.starts_with('#')) continue;
                    if(std::regex_match(line, matches, universal_pattern)){

                        obj o;
                        //1
                        o.obj_form = form::f_sphere;
                        //2
                        std::string coords = matches[2].str();
                        std::replace(coords.begin(), coords.end(), ',', ' ');
                        std::istringstream coord_stream(coords);
                        double x, y, z;
                        coord_stream >> x >> y >> z;
                        o.cords = point3(x, y, z);
                        coord_stream.clear();
                        //3
                        o.size = std::stod(matches[3].str());
                        //4
                        if(matches[4].str() == "metal"){ o.obj_mat = mat::m_metal; }
                        else if(matches[4].str() == "lambertian"){ o.obj_mat = mat::m_lambertian; }
                        else if(matches[4].str() == "dialetric"){o.obj_mat = mat::m_dialetric; }
                        //5
                        if(o.obj_mat == mat::m_dialetric){
                            o.dialetric = std::stod(matches[5].str());
                        }else{
                            std::string rgb_str = matches[5].str();
                            std::replace(rgb_str.begin(), rgb_str.end(), ',', ' ');
                            char l_paren, r_paren;
                            std::istringstream args_stream(rgb_str);
                            double r, g, b;
                            args_stream >> l_paren >> r >> g >> b >> r_paren;
                            o.rgb = color(r,g,b);
                            if(o.obj_mat == mat::m_metal) args_stream >> o.fuzz;
                        }
                        add_obj_to_hit_list(hit_list, o);
                    }
                }

                return hit_list;

            }catch(...){
                std::clog << "[Error] Exception while reading scene file!\n";
                return nullptr;
            }
        }

        inline camera* read_camera(std::filesystem::path src){
            if(!std::filesystem::is_regular_file(src) || src.extension() != ".tcam"){
                return nullptr;
            }
            std::ifstream file(src);
            if(!file.is_open()) return nullptr;
            try{
                camera* cam = new camera();
                std::string line;
                while(std::getline(file, line)){
                    std::istringstream input(line);
                    std::string obj, attr;
                    input >> obj >> attr;

                    if(obj == "aspect_ratio"){
                        cam->aspect_ratio = std::stod(attr);
                    }else if(obj == "image_width"){
                        cam->image_width = std::stod(attr);
                    }else if(obj == "samples_per_pixel"){
                        cam->samples_per_pixel = std::stod(attr);
                    }else if(obj == "max_depth"){
                        cam->max_depth = std::stod(attr);
                    }else if(obj == "vfov"){
                        cam->vfov = std::stod(attr);
                    }else if(obj == "defocus_angle"){
                        cam->defocus_angle = std::stod(attr);
                    }else if(obj == "focus_dist"){
                        cam->focus_dist = std::stod(attr);
                    }else {
                        std::istringstream vec(attr);
                        double x, y, z;
                        char l_paren, r_paren, comma;
                        vec >> l_paren >> x  >> comma >> y >> comma >> z >> r_paren;
                        if(obj == "lookfrom"){
                            cam->lookfrom = vec3(x, y, z);
                       }else if(obj == "lookat"){
                            cam->lookat = vec3(x, y, z);
                       }else if(obj == "vup"){
                            cam->vup = vec3(x, y, z);
                       }
                    }
                }
                return cam;
            }catch(...){
                return nullptr;
            }
        }

        inline renderer* read_renderer(std::string attr, config::cli_config& con){
            if(con.cam == nullptr){ return nullptr; }
            delete(con.r_renderer);
            if(attr == "concurrency"){ return new concurrency_driver(con.cam); }
            if(attr == "base"){ return new base_renderer(con.cam); }
            return nullptr;
        }
    }

    //=========================================

     inline bool read_config(config::cli_config& config){
        std::ifstream env_file(".env");
        if(!env_file.is_open()) return false;
        
        std::string src_path;
        std::getline(env_file, src_path);
        env_file.close();
        std::filesystem::path src(src_path);
        if(!std::filesystem::is_regular_file(src) || src.extension() != ".trt"){
            return false;
        }
        try{
            std::ifstream file(src);
            if(!file.is_open()) {
                return false;
            }
            std::string line;
            while(std::getline(file, line)){
                std::istringstream input(line);
                char q_marks;
                std::string obj, attr;
                input >> obj >> q_marks >> attr >> q_marks;
                if(obj == "Scene"){ config.scene = intern::read_scene(attr);}
                else if(obj == "Cam") {config.cam = intern::read_camera(attr); config.cam->initialize();}
                else if(obj == "Renderer") {config.r_renderer = intern::read_renderer(attr, config);}
            }
            return (config.scene != nullptr && config.cam != nullptr && config.r_renderer != nullptr);
        }catch(...){
            return false;
        }
    }

}