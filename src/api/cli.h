#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "../driver/multithreading.h"
#include "../driver/base_renderer.h"
#include "../reader/reader.h"
#include "config.h"

class cli {

    public:
    ~cli() {
        delete(con.cam);
        delete(con.scene);
    }

    int run(int argc, char* argv[]){
        std::string command = argv[1];
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);
        if (dispatch_map.contains(command)) {
            return dispatch_map.at(command)(argc, argv);
        } else {
            std::cerr << "[Error] Unknown command: " << command << std::endl;
            help();
            return 1;
        }
    }


    
    private:

    config::cli_config con;

    std::map<std::string, std::function<int(int args, char* argv[])>> dispatch_map = {
        {"-help", [this](int args, char* argv[]) {
            help();
            return 0;
        }},
        {"-scene--display", [this](int args, char* argv[]) {
            if(args != 1) return 1;
            return display_scene();
        }},
        {"-render", [this](int args, char* argv[]){
            //if(args != 2) return 1;
            return render(argv);
        }}
    };

    //Functions

    void help(){
        std::cout << "RayTracer Usage: "<<
        "\nRayTracer -render <dst>" << 
        "\nRayTracer -scene--display" <<
        "\n                 --erase" <<
        "\nRayTracer -camera--display" <<
        "\n                 --erase" <<
        "\nRayTracer -help";
    }

    int initialize(){
        try{
            if(!reader::read_config(con)){
                std::clog << "\n[Error] Could not read config.trt!";
                return 1;
            } 
            this->con.initialized = true;
            return 0;
        }catch(...){
            std::clog << "\n[Error] Could not read Initialize!";
            return 1;
        }

    }

    int render(char* argv[]){
        initialize();
        if(!this->con.initialized){
            std::clog << "[Error] RayTracer is not initialized!\n";
            return 1;
        }
        try{
            this->con.r_renderer->start_rendering(*this->con.scene);
            std::ofstream of(argv[2]);
            if(!of.is_open()) std::clog << "[Error] Couldnt open dst path!\n";
            this->con.r_renderer->print_rgbs(of);
            of.close();
        }catch(...){
            std::clog << "[Error] Oops, something went wrong!\n";
        }
        return 0;
    }

    int display_scene(){
        hittable* scene = reader::intern::read_scene("default_scene");
        if(scene == nullptr){
            std::clog << "[Error] Could not read default scene!\n";
            return 1;
        }
        for(auto& obj : static_cast<hittable_list*>(scene)->objects){
            std::clog << obj << "\n";
        }
        return 0;
    }
    
};