#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "../driver/concurrency_driver.h"
#include "../driver/base_renderer.h"
#include "../reader/reader.h"
#include "config.h"

namespace cli {

    namespace intern {

        //Functions

        void help(){
            std::cout << "RayTracer Usage: "<<
            "\nRayTracer -render <dst>" << 
            "\nRayTracer -run--test <renderer> <renderer>"
            "\nRayTracer -scene--display" <<
            "\nRayTracer -camera--display" <<
            "\nRayTracer -help";
        };

        int initialize(config::cli_config &con){
            try{
                if(!reader::read_config(con)){
                    std::clog << "\n[Error] Could not read config.trt!";
                    return 1;
                } 
                con.initialized = true;
                return 0;
            }catch(...){
                std::clog << "\n[Error] Could not read Initialize!";
                return 1;
            }

        };

        int render(char* argv[], config::cli_config &con){
            initialize(con);
            if(!con.initialized){
                std::clog << "[Error] RayTracer is not initialized!\n";
                return 1;
            }
            try{
                con.r_renderer->start_rendering(*con.scene);
                std::ofstream of(argv[2]);
                if(!of.is_open()) std::clog << "[Error] Couldnt open dst path!\n";
                con.r_renderer->print_rgbs(of);
                of.close();
            }catch(...){
                std::clog << "[Error] Oops, something went wrong!\n";
            }
            return 0;
        };

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
        };

        int run_test(int args, char* argv[], hittable* scene){
            //TODO
            return 1;
        };
    }

    const std::map<std::string, std::function<int(int args, char* argv[], config::cli_config &con)>> dispatch_map = {
        {"-help", [](int args, char* argv[], config::cli_config &con) {
            intern::help();
            return 0;
        }},
        {"-scene--display", [](int args, char* argv[], config::cli_config &con) {
            if(args != 1) return 1;
            return intern::display_scene();
        }},
        {"-render", [](int args, char* argv[], config::cli_config &con){
            if(args != 2) return 1;
            return intern::render(argv, con);
        }},
        {"-run--test", [](int args, char* argv[], config::cli_config &con){
            if(args != 4) return 1;
            return intern::run_test(args, argv, con.scene);
        }}
    };

    int run(int argc, char* argv[]){

        config::cli_config con;

        std::string command = argv[1];
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);
        if (dispatch_map.contains(command)) {
            return dispatch_map.at(command)(argc, argv, con);
        } else {
            std::cerr << "[Error] Unknown command: " << command << std::endl;
            intern::help();
            return 1;
        }
    };

    bool clear(config::cli_config &con){
        delete(con.cam);
        delete(con.r_renderer);
        delete(con.scene);
    };

    
    
}