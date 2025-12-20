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
        int width_start = (n % tiles::tiles_in_row) * tiles::size_sqrt;
        int height_start = (n / tiles::tiles_in_row) * tiles::size_sqrt;
        for (int j = height_start; j < height_start + tiles::size_sqrt; j++){
            for (int i = width_start; i < width_start + tiles::size_sqrt; i++){
                pixel_counter++;
                if(j >= static_cast<int>(cam->image_width / cam->aspect_ratio) || i >= cam->image_width){
                    curr.rgb[pixel_counter] = {-1, -1, -1};
                    continue;
                }
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
        tiles::calculate_tile_placement(&camera);

        size_t n_pixel = (cam->image_width * (cam->image_width/cam->aspect_ratio));
        worker_threads::num_tiles = tiles::tiles_in_column * tiles::tiles_in_row;
        rgbs.resize(num_tiles);

        tile_id.store(0);

        for(auto& worker : workers){
            worker = std::thread([&world]() {
                int current_tile;
                while((current_tile = get_next_tile_id()) < num_tiles){
                    std::clog << "Rendering tile " << current_tile << " / " << num_tiles << "\n" << std::flush;
                    render_tile(current_tile, world);
                }
            });
        }

        for(auto& worker : workers){
            if(worker.joinable()) worker.join();
        }
        
    }

    void print_rgbs(std::ostream& out){
        out << "P3\n" << cam->image_width << " " << cam->image_height << "\n255\n";
        for(int rows = 0; rows < tiles::tiles_in_row; rows++){
            for(int i = 0; i < tiles::size_sqrt; i++){
                for(int column = 0; column < tiles::tiles_in_column; column++){
                    //WTF
                }
            }
        }
    }
}