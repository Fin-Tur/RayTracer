cmake_minimum_required(VERSION 3.30)
project(RayTracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

# HIP Setup
set(ENABLE_HIP ON CACHE BOOL "Enable HIP GPU support")

if(ENABLE_HIP)
    if(NOT DEFINED ROCM_PATH)
        set(ROCM_PATH "/usr")
    endif()

    set(CMAKE_PREFIX_PATH "${ROCM_PATH};${CMAKE_PREFIX_PATH}")
    find_package(hip QUIET CONFIG)

    if(hip_FOUND)
        enable_language(HIP)
        set(CMAKE_HIP_STANDARD 20)
        message(STATUS "HIP found: ${hip_VERSION} - GPU support enabled")
        
        # Separate HIP Library für GPU-Code
        add_library(raytracer_hip STATIC src/driver/gpu_hip/gpu_driver.hip)
        target_link_libraries(raytracer_hip PUBLIC hip::host hip::device)
        target_include_directories(raytracer_hip PUBLIC ${CMAKE_SOURCE_DIR}/src)
        
        set(HIP_LIBRARY raytracer_hip)
    else()
        message(STATUS "HIP not found - GPU support disabled")
        set(HIP_LIBRARY "")
    endif()
else()
    message(STATUS "HIP support disabled (set ENABLE_HIP=ON to enable)")
    set(HIP_LIBRARY "")
endif()

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
     src/driver/concurrency_driver.h
     src/driver/performance.h
     src/api/cli.h
        src/reader/reader.h
)

# HIP library linken
if(ENABLE_HIP AND HIP_LIBRARY)
    target_link_libraries(RayTracer PRIVATE ${HIP_LIBRARY})
endif()
