#include "rtweekend.h"

#include "hittables/hittable.h"
#include "hittables/hittable_list.h"
#include "hittables/sphere.h"
#include "utils/camera.h"

int main() {

        //World
        hittable_list world;
        world.add(std::make_shared<sphere>(point3(0, 0, -1), 0.5));
        world.add(std::make_shared<sphere>(point3(0, -100.5, -1), 100));

        camera cam;

        cam.aspect_ratio = 16.0 / 9.0;
        cam.image_width = 400;
        cam.samples_per_pixel = 100;
        cam.render(world);
        
    return 0;
}