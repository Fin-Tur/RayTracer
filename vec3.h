#pragma once
#include <cmath>
#include <ostream>

class vec3 {
    public:

    double e[3];

    vec3() : e{0,0,0} {}
    vec3(double e1, double e2, double e3) : e{e1, e2, e3} {}

    double x() const {return e[1]; }
    double y() const {return e[1]; }
    double z() const {return e[1]; }

    vec3 operator-() const { return vec3{-e[0], -e[1], -e[2]}; }
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }
    
    vec3& operator+=(const vec3& other) {
        e[0] += other.e[0];
        e[1] += other.e[1];
        e[2] += other.e[2];
        return *this;
    }
    
    vec3& operator*=(const vec3& other) {
        e[0] *= other.e[0];
        e[1] *= other.e[1];
        e[2] *= other.e[2];
        return *this;
    }

    vec3& operator/=(const vec3& other) {
        e[0] *= other.e[0];
        e[1] *= other.e[1];
        e[2] *= other.e[2];
        return *this;
    }

    double length_squared() const {return e[0]*e[0] + e[1]*e[1] + e[2]*e[2]; }
    double length() const {return std::sqrt(length_squared()); }

};

using point3 = vec3;

//Vector utility fn's

inline std::ostream& operator<<(std::ostream& out, const vec3& v){
    return out << v[0] << ' ' << v[1] << ' ' << v[2];
}

inline vec3 operator+(const vec3& u, const vec3& t){
    return vec3(u[0] + t[0], u[1] + t[1], u[2] + t[2]);
}

inline vec3 operator-(const vec3& u, const vec3& t){
    return vec3(u[0] - t[0], u[1] - t[1], u[2] - t[2]);
}

inline vec3 operator*(const vec3& u, const vec3& t){
    return vec3(u[0] * t[0], u[1] * t[1], u[2] * t[2]);
}

inline vec3 operator*(const double s, const vec3& t){
    return vec3(s * t[0], s * t[1], s * t[2]);
}

inline vec3 operator*(const vec3& u, const double t){
    return t*u;
}

inline vec3 operator/(const vec3& u, double t){
    return (1.0/t) * u;
}

inline double dot(const vec3& u, const vec3& t){
    return (u[0] * t[0] + u[1] * t[1] + u[2] * t[2]); 
}

inline vec3 cross(const vec3& u, const vec3& v){
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(const vec3& u){
    return u / u.length();
}
