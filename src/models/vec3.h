#pragma once

#if defined(__CUDACC__) || defined(__HIP_PLATFORM_AMD__)
#define HD __host__ __device__
#define H __host__ 
#define D __device__
#include <hip/hip_runtime.h>
#else
#define HD
#define H 
#define D
#endif

#include "../rtweekend.h"
#include "../driver/gpu_hip/rng__seeded.h"



class vec3 {
    public:

    double e[3];

    HD vec3() : e{0,0,0} {}
    HD vec3(double e1, double e2, double e3) : e{e1, e2, e3} {}

    HD double x() const {return e[0]; }
    HD double y() const {return e[1]; }
    HD double z() const {return e[2]; }

    HD vec3 operator-() const { return vec3{-e[0], -e[1], -e[2]}; }
    HD double operator[](int i) const { return e[i]; }
    HD double& operator[](int i) { return e[i]; }
    
    HD vec3& operator+=(const vec3& other) {
        e[0] += other.e[0];
        e[1] += other.e[1];
        e[2] += other.e[2];
        return *this;
    }
    
    HD vec3& operator*=(const vec3& other) {
        e[0] *= other.e[0];
        e[1] *= other.e[1];
        e[2] *= other.e[2];
        return *this;
    }

    HD vec3& operator/=(const vec3& other) {
        e[0] /= other.e[0];
        e[1] /= other.e[1];
        e[2] /= other.e[2];
        return *this;
    }

    HD bool operator==(const vec3& other) const{
        return (other.e[0] == e[0] && other.e[1] == e[1] && other.e[2] == e[2]);
    }

    HD double length_squared() const {return e[0]*e[0] + e[1]*e[1] + e[2]*e[2]; }

    HD double length() const {return sqrtf(length_squared()); }

    static H vec3 random(){
        return vec3(random_double(), random_double(), random_double());
    }

    static D vec3 random_rng(rng::rng_state& rng){
        return vec3(rng::next_double(rng), rng::next_double(rng), rng::next_double(rng));
    }

    static H vec3 random(double min, double max){
        return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }

    static D vec3 random_rng_range(rng::rng_state& rng, double min, double max){
        return vec3(rng::next_double_range(rng, min, max), rng::next_double_range(rng, min, max), rng::next_double_range(rng, min, max));
    }

    HD bool near_zero() const {
        auto s = 1e-8;
        return (fabs(e[0]) < s && fabs(e[1]) < s && fabs(e[2]) < s);
    }

    

};

using point3 = vec3;

//Vector utility fn's

H inline std::ostream& operator<<(std::ostream& out, const vec3& v){
    return out << v[0] << ' ' << v[1] << ' ' << v[2];
}

HD inline vec3 operator+(const vec3& u, const vec3& t){
    return vec3(u[0] + t[0], u[1] + t[1], u[2] + t[2]);
}

HD inline vec3 operator-(const vec3& u, const vec3& t){
    return vec3(u[0] - t[0], u[1] - t[1], u[2] - t[2]);
}

HD inline vec3 operator*(const vec3& u, const vec3& t){
    return vec3(u[0] * t[0], u[1] * t[1], u[2] * t[2]);
}

HD inline vec3 operator*(const double s, const vec3& t){
    return vec3(s * t[0], s * t[1], s * t[2]);
}

HD inline vec3 operator*(const vec3& u, const double t){
    return t*u;
}

HD inline vec3 operator/(const vec3& u, double t){
    return (1.0/t) * u;
}

HD inline double dot(const vec3& u, const vec3& t){
    return (u[0] * t[0] + u[1] * t[1] + u[2] * t[2]); 
}

HD inline vec3 cross(const vec3& u, const vec3& v){
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

HD
inline vec3 unit_vector(const vec3& u){
    return u / u.length();
}

inline vec3 random_unit_vector() {
    while(true){
        auto p = vec3::random(-1,1);
        auto lensq = p.length_squared();
        if(lensq > 1e-160 && lensq <= 1){
            return p / sqrtf(lensq);
        }
    }
}

inline vec3 random_in_unit_disk(){
    while (true){
        auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
        if (p.length_squared() < 1){
            return p;
        }
    }
}

inline vec3 random_on_hemisphere(const vec3& normal){
    vec3 on_unit_sphere = random_unit_vector();
    return dot(on_unit_sphere, normal) > 0.0 ? on_unit_sphere : -on_unit_sphere;
}

HD inline vec3 reflect(const vec3& v, const vec3& n){
    return v - 2*dot(v, n)*n;
}

HD inline vec3 refract(const vec3& uv, const vec3& n, double etai_over_etat){
    auto cos_theta = fmin(dot(-uv, n), 1.0);
    vec3 r_out_perp = etai_over_etat * (uv + cos_theta*n);
    vec3 r_out_parallel = -sqrtf(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}