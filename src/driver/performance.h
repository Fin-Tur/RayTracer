#pragma once
#include <cstdint>
#include <functional>
#include <concepts>
#include <chrono>
#include <memory>
#include "../rtweekend.h"
#include "../utils/camera.h"
#include "renderer.h"

//Returns 

namespace performance_tests{
    
    template<typename F>
    concept render_function = requires(F f, const hittable& world){
        f(world);
    };

    template<render_function F, render_function G>
    auto compare_render_functions(F&& f, G&& g, const hittable &world) -> std::pair<uint64_t, uint64_t>{

        auto start_f = std::chrono::high_resolution_clock::now();
        f(world);
        auto end_f = std::chrono::high_resolution_clock::now();

        auto start_g = std::chrono::high_resolution_clock::now();
        g(world);
        auto end_g = std::chrono::high_resolution_clock::now();

        auto time_f = std::chrono::duration_cast<std::chrono::nanoseconds>(end_f - start_f).count();
        auto time_g = std::chrono::duration_cast<std::chrono::nanoseconds>(end_g - start_g).count();    
    
        return std::make_pair<>(time_f, time_g);
    }

    auto compare_render_functions(std::unique_ptr<renderer> r1, std::unique_ptr<renderer> r2, const hittable& world) -> std::pair<uint64_t, uint64_t>{
        return compare_render_functions(
            [&r1, &world](const hittable&) { r1->start_rendering(world); },
            [&r2, &world](const hittable&) { r2->start_rendering(world); },
            world
        );
    }
}