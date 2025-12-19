#pragma once

#include "../utils/camera.h"

namespace tiles{

    //width, height
    static int multiplier[2];

    struct tile
    {
        int num;
        color rgb[256];
    };

    void calculate_tile_placement (const camera* cam){
        tiles::multiplier[0] = cam->image_width / 16;
        tiles::multiplier[1] = (cam->image_width / cam->aspect_ratio) / 16;
    }
}