#pragma once

#include "../hittables/hittable.h"
#include "../rtweekend.h"

class material {
    public:
    virtual ~material() = default;
    virtual bool scatter (const ray& r_in, const hit_record& rec, color& attentuation, ray& scattered) const {
        return false;
    }
};

class lambertian : public material {
    public:
    lambertian(const color& albedo) : albedo(albedo) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attentuation, ray& scattered) const override {
        auto scatter_direction = rec.normal + random_unit_vector();

        if(scatter_direction.near_zero()) scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);
        attentuation = albedo;
        return true;
    } 

    private:
    color albedo;
};

class metal : public material {
    public:
    metal(const color& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool scatter(const ray &r_in, const hit_record &rec, color &attentuation, ray &scattered) const override {
        vec3 reflected = reflect(r_in.direction(), rec.normal);
        reflected = unit_vector(reflected) + (fuzz * random_unit_vector());
        scattered = ray(rec.p, reflected);
        attentuation = albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }

    private:
    color albedo;
    double fuzz;
};

class dialectric : public material {
    public:
    dialectric(double refraction_index) : refraction_index(refraction_index) {}

    bool scatter(const ray &r_in, const hit_record &rec, color &attentuation, ray &scattered) const override{
        attentuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0/refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());
        vec3 refracted = refract(unit_direction, rec.normal, ri);

        scattered = ray(rec.p, refracted);
        return true;
    }
        
    private:
    color attentuation;
    //refractive index in vaccum or air, or the ratio of the materials refractuve index over the refrc. index of the enclosing media
    double refraction_index;
};