cmake_minimum_required(VERSION 3.30)
project(RayTracer)

set(CMAKE_CXX_STANDARD 20)

add_executable(RayTracer
     src/main.cpp
     src/models/vec3.h
     src/models/color.h
     src/models/ray.h
     src/hittables/hittable.h
     src/hittables/sphere.h
)
