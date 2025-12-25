#pragma once

#include "../hittables/hittable.h"
#include "../utils/camera.h"

#include <filesystem>
#include <optional>

namespace reader {

    inline hittable* read_scene(std::filesystem::path src){
        if(!std::filesystem::is_regular_file(src) || std::filesystem::equivalent(src.extension(), std::filesystem::path(".tsc"))){
            return nullptr;
        }

        try{
            //TODO
            std::string line;
            
        }catch(...){
            return nullptr;
        }
    }

    inline camera* read_camera(std::filesystem::path src){

    }
}