cmake_minimum_required(VERSION 3.30)
project(RayTracer)

set(CMAKE_CXX_STANDARD 20)

add_executable(RayTracer
     src/main.cpp
     src/models/vec3.h
     src/models/color.h
     src/models/ray.h
     src/models/material.h
     src/models/interval.h
     src/models/tile.h
     src/hittables/hittable.h
     src/hittables/sphere.h
     src/hittables/hittable_list.h
     src/utils/camera.h
     src/driver/base_renderer.h
     src/driver/renderer.h
     src/driver/multithreading.h
     src/driver/performance.h
     src/api/cli.h
)
