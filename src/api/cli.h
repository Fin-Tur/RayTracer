#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <map>
#include <iostream>

class cli {

    cli() = default;
    
    private:

    std::map<std::string, std::function<int(int args, char* argv[])>> f_map = {
        {"help", [this](int args, char* argv[]) {
            help();
            return 0;
        }}
    };

    void help(){
        std::cout << "Usage: \nRayTracer -render <dst>\nRayTracer -set--scene <src>\nRayTracer -set--renderer <concurrency/base>\n";
    }
};