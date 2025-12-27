#pragma once

#include "../driver/renderer.h"
#include "../utils/camera.h"
#include "../hittables/hittable_list.h"
#include <memory>
#include <filesystem>

namespace config{

    struct cli_config{
        renderer* renderer;
        camera* cam;
        hittable* scene;
        std::filesystem::path dst;
        bool initialized;
    };

    inline int config_set(cli_config& c) noexcept{
        return (c.renderer!=nullptr && c.cam!=nullptr && c.scene!=nullptr && !c.dst.empty());
    }

    inline void save_config(cli_config& c){
        //TODO
    }

    inline bool read_config(std::filesystem::path src){
        //TODO
        return false;
    }
    
}

