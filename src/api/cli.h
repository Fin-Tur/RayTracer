#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

#include "../driver/multithreading.h"
#include "../driver/base_renderer.h"
#include "../reader/reader.h"
#include "config.h"

class cli {

    cli() = default;

    ~cli() {

    }

    config::cli_config con;
    
    private:

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
        "\nRayTraver -initialize" <<
        "\nRayTracer -render <dst>" << 
        "\nRayTracer -set--scene <src.tsc>" << 
        "\n              --cam <src.tcam>" <<
        "\n              --renderer <concurrency/base>\n" <<
        "\nRayTracer -config--display" <<
        "\n                 --erase" <<
        "\nRayTracer -help";
    }

    int initialize(){
        try{
            if(!reader::read_config("C:/Users/f.willems/config.trt", con)) return 1;
            this->con.initialized = true;
            return 0;
        }catch(...){
            std::clog << "\n[Error] Could not read config.trt!";
            return 1;
        }

    }

    int render(char* argv[]){
        if(!this->con.initialized){
            std::clog << "[Error] RayTracer is not initialized!\n";
            return 1;
        }
        try{
            std::clog << "\nStarting to render: ...";
            this->con.renderer->start_rendering(*this->con.scene);
            std::ofstream of(argv[1]);
            if(!of.is_open()) std::clog << "[Error] Couldnt open dst path!\n";
            std::clog << "\nPrintig RGBs!";
            this->con.renderer->print_rgbs(of);
            std::clog << "\nDone!";
        }catch(...){
            std::clog << "[Error] Oops, something went wrong!\n";
        }
    }

    //FIXME
    int set_renderer(char* argv[]){
        renderer* renderer;
        if(argv[1] == "concurrency"){
            renderer = new concurrency_driver(this->con.cam);
        }else if(argv[1] == "base"){
            renderer = new base_renderer(this->con.cam);
        }
        this->con.renderer = renderer;
        return 0;
    }

    int set_scene(char* argv[]){
        //TODO
        this->con.initialized = false;
    }

    int set_cam(char* argv[]){
        //TODO
        this->con.initialized = false;
    }

    int show_config(){
        //TODO
    }

    int erase_config(){
        //TODO
        this->con.initialized = false;
    }

    int save_config() {
        //TODO
    }
};