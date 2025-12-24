#pragma once

#include "../driver/renderer.h"
#include "../utils/camera.h"
#include "../hittables/hittable_list.h"
#include <memory>
#include <filesystem>

struct cli_config{
    std::unique_ptr<renderer> renderer;
    camera cam;
    hittable_list world;
    std::filesystem::path dst;
};