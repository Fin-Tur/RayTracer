#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <map>
#include <iostream>

#include "../driver/multithreading.h"
#include "../driver/base_renderer.h"
#include "config.h"

class cli {

    cli() = default;
    
    private:

    config::cli_config con;

    std::map<std::string, std::function<int(int args, char* argv[])>> f_map = {
        {"-help", [this](int args, char* argv[]) {
            help();
            return 0;
        }},
        {"-set--renderer", [this](int args, char* argv[]) {
            if(args != 2) return 1;
            return set_renderer(argv);
        }}
    };

    //Functions

    void help(){
        std::cout << "RayTracer Usage: "<<
        "\nRayTracer -render <dst>" << 
        "\nRayTracer -set--scene <src.tsc>" << 
        "\n              --cam <src.tcam>" <<
        "\n              --renderer <concurrency/base>\n" <<
        "\nRayTracer -config--display" <<
        "\n                 --erase" <<
        "\nRayTracer -help";
    }

    int render(char* argv[]){
        //TODO
    }

    int set_renderer(char* argv[]){
        if(argv[1] == "concurrency"){
            this->con.renderer = std::make_shared<renderer>(concurrency_driver(this->con.cam));
            return 0;
        }else if(argv[1] == "base"){
            this->con.renderer = std::make_shared<renderer>(base_renderer(this->con.cam));
            return 0;
        }
    }

    int set_scene(char* argv[]){
        //TODO
    }

    int set_cam(char* argv[]){
        //TODO
    }

    int show_config(){
        //TODO
    }

    int erase_config(){
        //TODO
    }

    int save_config() {
        //TODO
    }
};