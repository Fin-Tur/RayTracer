#pragma once
#include "../rtweekend.h"
#include "../utils/camera.h"
#include "../models/tile.h"
#include <atomic>
#include <thread>

namespace worker_threads {

    static std::vector<tiles::tile> rgbs;
    static int num_threads = std::thread::hardware_concurrency();
    static size_t num_tiles;
    static std::vector<std::thread> workers(num_threads-1);
    std::atomic<int> tile_id{0};

    static const camera* cam = nullptr;

    int get_next_tile_id(){
        return tile_id.fetch_add(1);
    }

    void render_tile(int n, const hittable& world){
        tiles::tile curr = {n};
        int pixel_counter = -1;
        for(int i = tiles::multiplier[0] * n; i < std::min(tiles::multiplier[0] * (n+1), cam->image_width-1); i++){
            for (int j = tiles::multiplier[1] * n; j < std::min(tiles::multiplier[1] * (n + 1), static_cast<int>(cam->image_width/cam->aspect_ratio) - 1); j++){
                pixel_counter++;
                color pixel_color(0, 0, 0);
                for (int sample = 0; sample < cam->samples_per_pixel; sample++){
                    ray r = cam->get_ray(i, j);
                    pixel_color += cam->ray_color(r, cam->max_depth, world);
                }
                curr.rgb[pixel_counter] = cam->pixel_samples_scale * pixel_color;
                //write_color(std::cout, cam->pixel_samples_scale * pixel_color);
            }
        }
        worker_threads::rgbs[n] = curr;
    }

    void start_rendering(const camera &camera, const hittable& world)
    {
        worker_threads::cam = &camera;
        size_t n_pixel = (cam->image_width * (cam->image_width/cam->aspect_ratio));
        num_tiles = n_pixel/(16*16);
        rgbs.resize(num_tiles);

        tiles::calculate_tile_placement(&camera);
        tile_id.store(0);

        for(auto& worker : workers){
            worker = std::thread([&world]() {
                int current_tile;
                while((current_tile = get_next_tile_id()) < num_tiles){
                    render_tile(current_tile, world);
                }
            });
        }

        for(auto& worker : workers){
            if(worker.joinable()) worker.join();
        }
        
    }
}