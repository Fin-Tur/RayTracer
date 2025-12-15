#pragma once

#include "../rtweekend.h"
#include "../hittables/hittable.h"
#include "../models/material.h"

class camera {
  public:

   //image
    double aspect_ratio = 1.0; //Ratio of image width over height
    int image_width = 100; 
    int samples_per_pixel = 10; //Count of random smples for each pixel
    int max_depth = 10; // Maximum number of ray bounces into scene

    double vfov = 90; //Vertical view angle (field of view)
    point3 lookfrom = point3(0,0,0); // Point of camera
    point3 lookat = point3(0,0,-1); //Point camera is lookint at
    vec3 vup = vec3(0,1,0); // "Forward tilt" of the camera || "up" position from camera

    void render(const hittable& world){
    initialize();
    std::cout << "P3\n" << image_width << " " << image_height << "\n255\n";

        for(int j = 0; j < image_height; j++){
            std::clog << "\rScanlines remaining: " << image_height - j - 1 << ' ' << std::flush;
            for(int i = 0; i < image_width; i++){
              color pixel_color(0,0,0);
              for(int sample = 0; sample < samples_per_pixel; sample++){
                auto curr_ray = get_ray(i, j);
                pixel_color += ray_color(curr_ray, max_depth ,world);
              } 
              write_color(std::cout, pixel_samples_scale * pixel_color);
            }
        }
        std::clog << "\nDone.\n";
  }

  private:

  int image_height; //Rendered image hieght
  point3 center;    //Camera center
  double pixel_samples_scale;
  point3 pixel00_loc;  //Location of pixel 0,0
  vec3 pixel_delta_u; //Offset to pixel to the right
  vec3 pixel_delta_v; //Offset to pixel below
  vec3 u, v, w; // Camera frame base vectors

  void initialize(){

    image_height = std::max(int(image_width/aspect_ratio), 1);

    center = lookfrom;

    pixel_samples_scale = 1.0 / samples_per_pixel;

    //Determine viewport dimension
    auto focal_length = (lookfrom - lookat).length();
    auto theta = degrees_to_radians(vfov);
    auto h = std::tan(theta/2);
    auto viewport_height = 2.0 * h * focal_length;
    auto viewport_width = viewport_height * (double(image_width)/image_height);

    //Calculate u, v, w unit basis vectors for the camera coordinate frame.
    w = unit_vector(lookfrom-lookat);
    u = unit_vector(cross(vup, w));
    v = cross(w, u);

    //Calc vectors accros horizinatl and down the vertical viewport edges
    auto viewport_u = viewport_width * u;
    auto viewport_v = viewport_height * -v;

    //Calc the horizintal and verical delta vectors from pixel to pixel
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;

    //Calculate loc of the upper ledt pixel
    auto viewport_upper_left = center - (focal_length*w) - viewport_u/2 - viewport_v/2;
    pixel00_loc = viewport_upper_left + 0.5*(pixel_delta_u + pixel_delta_v);

  }

  ray get_ray(int i, int j) const{
    //Construct a camera ray origninating from the origin and directed at randomly samples point around the pixel location i,j
    auto offset = sample_square();
    auto pixel_sample = pixel00_loc + ((i+offset.x()) * pixel_delta_u) + ((j+offset.y()) * pixel_delta_v);

    auto ray_origin = center;
    auto ray_direction = pixel_sample - ray_origin;

    return ray(ray_origin, ray_direction);
  }

  vec3 sample_square() const {
    //Returns the vector to a random point in the [-.5, -.5] - [-5, .5] unit square
    return vec3(random_double() - 0.5, random_double() -0.5, 0);
  }

  color ray_color(const ray& r, int depth, const hittable& world) const {

    if(depth < 1){
      return color(0,0,0);
    }

    hit_record rec;

    if (world.hit(r, interval(0.001, infinity), rec)){
        ray scattered;
        color attentuation;
        if(rec.mat -> scatter(r, rec, attentuation, scattered)) return attentuation * ray_color(scattered, depth-1, world);
        return color(0,0,0);
    }

    vec3 unit_direction = unit_vector(r.direction());
    auto a = 0.5*(unit_direction.y() + 1.0);
    return (1.0-a) * color(1.0, 1.0, 1.0) + a*color(0.5, 0.7, 1.0);

  }
};