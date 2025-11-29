cmake_minimum_required(VERSION 3.30)
project(RayTracer)

set(CMAKE_CXX_STANDARD 20)

add_executable(RayTracer
     main.cpp
     vec3.h
     color.h
     ray.h
)
