#pragma once
#include "../rtweekend.h"
#include "../utils/camera.h"
#include "../models/tile.h"
#include "renderer.h"
#include <atomic>
#include <thread>
#include <iostream>

class concurrency_driver : public renderer{

    public:

        concurrency_driver(const std::shared_ptr<camera> camera, int tile_size_sqrt = 16, int num_threads = std::thread::hardware_concurrency()) : renderer(camera){
            this->num_threads = num_threads;
            tiles::calculate_tile_placement(this->t_ctx, cam, tile_size_sqrt);
            this->num_tiles = t_ctx.tiles_in_column * t_ctx.tiles_in_row;
            this->num_pixels = (cam->image_width * cam->image_height);
            this->frame_buffer.resize(num_pixels);
            this->workers.resize(this->num_threads);
        }

        void start_rendering(const hittable &world) override{
            tile_id.store(0);

            for (auto &worker : workers){
                worker = std::thread([this, &world](){
                int current_tile;
                while((current_tile = get_next_tile_id()) < num_tiles){
                    render_tile(current_tile, world);
                } });
            }

            int current_progress;
            while((current_progress = get_current_tile_id()) < num_tiles){
                std::clog << "\rRendering: " << int((float(current_progress)/num_tiles)*100) << "% "  << std::flush;   
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            for (auto &worker : workers){
                if (worker.joinable())
                    worker.join();
            }
        }


    private:

        tiles::tile_ctx t_ctx;
        int num_threads; 
        size_t num_tiles;
        size_t num_pixels;
        std::vector<std::thread> workers;
        std::atomic<int> tile_id{0};
        

        int get_next_tile_id(){
            return tile_id.fetch_add(1);
        }

        int get_current_tile_id(){
            return tile_id.load();
        }

    void render_tile(int n, const hittable &world){
            color* fb = frame_buffer.data();

            int width_start = (n % t_ctx.tiles_in_row) * t_ctx.size_sqrt;
            int height_start = (n / t_ctx.tiles_in_row) * t_ctx.size_sqrt;
            for (size_t j = height_start; j < height_start + t_ctx.size_sqrt; j++){
                for (size_t i = width_start; i < width_start + t_ctx.size_sqrt; i++){
                    size_t pixel = j*cam->image_width + i;
                    if (j >= cam->image_height || i >= cam->image_width){
                        continue;
                    }
                    color pixel_color(0, 0, 0);
                    for (int sample = 0; sample < cam->samples_per_pixel; sample++){
                        ray r = cam->get_ray(i, j);
                        pixel_color += cam->ray_color(r, cam->max_depth, world);
                    }
                    fb[pixel] = cam->pixel_samples_scale * pixel_color;
                    }
                }
            }
};