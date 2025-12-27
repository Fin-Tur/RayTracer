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

namespace reader {

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
                if(std::regex_match(line, matches, universal_pattern)){

                    obj o; 
                    //1
                    o.obj_form = form::f_sphere;
                    //2
                    std::istringstream coord_stream(matches[2].str());
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
                        char l_paren, r_paren;
                        std::istringstream args_stream(matches[5].str());
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
            return nullptr;
        }
    }

    inline camera* read_camera(std::filesystem::path src){
        //TODO
    }

    inline renderer* read_renderer(std::string attr, config::cli_config& con){
        if(con.cam == nullptr){ return nullptr; }
        if(attr == "concurrency"){ return new concurrency_driver(con.cam); }
        if(attr == "base"){ return new base_renderer(con.cam); }
    }

    inline bool read_config(std::filesystem::path src, config::cli_config& config){
        if(!std::filesystem::is_regular_file(src) || src.extension() != ".tsc"){
            return false;
        }
        try{
            std::ifstream file(src);
            if(!file.is_open()) return false;
            std::string line;
            while(std::getline(file, line)){
                std::istringstream input(line);
                char q_marks;
                std::string obj, attr;
                input >> obj >> q_marks >> attr >> q_marks;
                
                if(obj == "Scene"){ config.scene = read_scene(attr); }
                else if(obj == "Cam") {config.cam = read_camera(attr);}
                else if(obj == "Renderer") {config.renderer = read_renderer(attr, config); }
            }
            return (config.scene != nullptr && config.cam != nullptr && config.renderer != nullptr);
        }catch(...){
            return false;
        }
    }
}