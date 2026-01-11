cmake_minimum_required(VERSION 3.21)
project(RayTracer LANGUAGES CXX HIP)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(HIP REQUIRED)

# Projekt-Include (damit #include "driver/..." etc. funktioniert)
set(PROJECT_INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# -------------------------
# Gemeinsame Dateien (optional: Header nur für IDE-Sichtbarkeit)
# -------------------------
set(RAY_HEADERS
  src/utils/camera.h
  src/reader/reader.h
  src/models/ray.h
  src/models/color.h
  src/models/interval.h
  src/models/material.h
  src/models/tile.h
  src/models/vec3.h
  src/hittables/hittable.h
  src/hittables/hittable_list.h
  src/hittables/sphere.h
  src/driver/base_renderer.h
  src/driver/performance.h
  src/driver/renderer.h
  src/driver/concurrency_driver.h
  src/driver/gpu_hip/gpu_driver.h
  src/driver/gpu_hip/rng__seeded.h
  src/api/config.h
  src/api/cli.h
)

# -------------------------
# GPU: HIP library (NO main.cpp)
# -------------------------
add_library(ray_gpu STATIC
  src/driver/gpu_hip/gpu_driver.hip
  src/driver/gpu_hip/gpu_driver.h
  src/driver/gpu_hip/rng__seeded.h
)

# .hip wirklich als HIP kompilieren
set_source_files_properties(src/driver/gpu_hip/gpu_driver.hip PROPERTIES LANGUAGE HIP)

# Architektur (zum Start "native", später z.B. "gfx1031" etc.)
set_target_properties(ray_gpu PROPERTIES HIP_ARCHITECTURES "native")

# HIP runtime / device lib
target_link_libraries(ray_gpu PRIVATE hip::device)

# Include-Pfad für GPU lib
target_include_directories(ray_gpu PUBLIC ${PROJECT_INC_DIR})

# -------------------------
# CPU: Hauptprogramm (hat main.cpp)
# -------------------------
add_executable(ray
  src/main.cpp
  ${RAY_HEADERS}   # optional, nur damit es in der IDE auftaucht
)

target_include_directories(ray PRIVATE ${PROJECT_INC_DIR})

# CPU-Programm linkt gegen GPU-Library -> du kannst GPU-Funktionen normal aufrufen
target_link_libraries(ray PRIVATE ray_gpu)

set_target_properties(ray PROPERTIES POSITION_INDEPENDENT_CODE ON)

