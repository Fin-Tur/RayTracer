#pragma once

#include "../utils/camera.h"

namespace tiles{

    struct tile_ctx{
        int size_sqrt;
        int tiles_in_row;
        int tiles_in_column;
    };

    inline void calculate_tile_placement (tile_ctx& ctx, const camera* const cam, int tile_size){
        ctx.size_sqrt = tile_size;
        ctx.tiles_in_row = (cam->image_width + tile_size - 1) / tile_size;
        ctx.tiles_in_column = (cam->image_height + tile_size - 1) / tile_size;
        }
}