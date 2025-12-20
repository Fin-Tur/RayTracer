#pragma once

#include "../utils/camera.h"

namespace tiles{

    static int size_sqrt;
    static int tiles_in_row;
    static int tiles_in_column;

    struct tile
    {
        int num;
        color rgb[256];
    };

    void calculate_tile_placement (const camera* cam, int tile_size = 16){
        size_sqrt = tile_size;
        tiles_in_row = (cam->image_width + tile_size - 1) / tile_size;
        tiles_in_column = ((cam->image_width / cam->aspect_ratio) + tile_size - 1) / tile_size;
        }
}