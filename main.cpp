#include <iostream>

#include "vec3.h"
#include "color.h"

int main() {

        //image
        int image_height = 256;
        int image_width = 256;

        //Render
        std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for(int j = 0; j < image_height; j++){
            std::clog << "\rScanlines remaining: " << image_height - j - 1 << ' ' << std::flush;
            for(int i = 0; i < image_width; i++){
                auto r = double(i) / (image_width-1);
                auto g = double(j) / (image_height-1);
                auto b = 0.0;

                const color c = color(r, g, b);
                write_color(std::cout, c);     
                
            }
        }

        std::clog << "\nDone.\n";
    return 0;
}