#pragma once
#include "../rtweekend.h"
#include "../utils/camera.h"
#include "../models/tile.h"
#include <atomic>
#include <thread>

namespace worker_threads {

    static std::vector<tile> rgbs;
    static int num_threads = std::thread::hardware_concurrency();
    static std::vector<std::thread> workers(num_threads-1);
    std::atomic<int> tile_id{0};

    static const camera* cam = nullptr;

    int get_next_tile_id(){
        return tile_id.fetch_add(1);
    }

    void render_tile(int n){
        
    }

    void start_rendering(const camera &camera, const hittable & world)
    {
        worker_threads::cam = &camera;
        size_t n_pixel = (cam->image_width * (cam->image_width/cam->aspect_ratio));
        size_t n_tiles = n_pixel/(16*16);
        rgbs.resize(n_tiles);

        tile_id.store(0);

        for(auto& worker : workers){
            int tile_id;
            worker = std::thread([&world]() {
                int current_tile;
                while(current_tile = get_next_tile_id() < num_threads){
                    render_tile(current_tile);
                }
            });
        }
        
    }
}