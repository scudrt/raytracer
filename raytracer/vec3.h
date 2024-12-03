#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>

#include <intrin.h>

using std::sqrt;

class vec3 {
    public:
        vec3() {}
        vec3(float e0, float e1, float e2) : e{e0, e1, e2, 0.0f} {}

        inline float x() const { return e[0]; }
        inline float y() const { return e[1]; }
        inline float z() const { return e[2]; }
		inline float w() const { return e[3]; }

        vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
        float operator[](int i) const { return e[i]; }
        float& operator[](int i) { return e[i]; }

        bool near_zero() const {
			// Return true if the vector is close to zero in all dimensions.
			const auto s = 1e-6;
			return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
		}

		vec3 operator+(float x) {
			return *this - (-x);
		}

		vec3 operator-(float x) {
			return vec3(e[0] - x, e[1] - x, e[2] - x);
		}

        vec3& operator+=(const vec3 &v) {
            e[0] += v.e[0];
            e[1] += v.e[1];
            e[2] += v.e[2];
            return *this;
        }

		vec3& operator*=(const float t) {
            __m128 a = _mm_load_ps(e);
            __m128 b = _mm_load_ps1(&t);
            a = _mm_mul_ps(a, b);
			_mm_store_ps(e, a);
			return *this;
		}

		vec3& operator*=(const vec3& other) {
			_mm_store_ps(e, _mm_mul_ps(_mm_load_ps(e), _mm_load_ps(other.e)));
			return *this;
		}

        vec3& operator/=(const float t) {
            return *this *= 1/t;
        }

        float length() const {
            return sqrt(length_squared());
        }

        float length_squared() const {
			float r[4] = { 0, 0, 0, 0 };
			__m128 a = _mm_load_ps(e);
			_mm_store_ps(r, _mm_mul_ps(a, a));
			return r[0] + r[1] + r[2];
        }

    public:
        float e[4];
};

// Type aliases for vec3
using point3 = vec3;   // 3D point
using color = vec3;    // RGB color
// vec3 Utility Functions

inline std::ostream& operator<<(std::ostream &out, const vec3 &v) {
    return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2]);
}

inline vec3 operator-(const vec3 &u, const vec3 &v) {
    return vec3(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2]);
}

inline vec3 operator*(const vec3 &u, const vec3 &v) {
	vec3 ret;
    __m128 a = _mm_load_ps(u.e);
    __m128 b = _mm_load_ps(v.e);
    a = _mm_mul_ps(a, b);
	_mm_store_ps(ret.e, a);
	return ret;
}

inline vec3 operator*(float t, const vec3 &v) {
	vec3 ret;
    __m128 a = _mm_load_ps(v.e);
    __m128 b = _mm_load_ps1(&t);
    a = _mm_mul_ps(a, b);
	_mm_store_ps(ret.e, a);
	return ret;
}

inline vec3 operator*(const vec3 &v, float t) {
	vec3 ret;
    __m128 a = _mm_load_ps(v.e);
    __m128 b = _mm_load_ps1(&t);
    a = _mm_mul_ps(a, b);
    _mm_store_ps(ret.e, a);
	return ret;
}

inline vec3 operator/(vec3 v, float t) {
    return v * (1.0f / t);
}

inline float dot(const vec3 &u, const vec3 &v) {
	return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2];
	float ret[4] = { 0, 0, 0, 0 };
	_mm_store_ps(ret, _mm_mul_ps(_mm_load_ps(u.e), _mm_load_ps(v.e)));
	return ret[0] + ret[1] + ret[2];
}

inline vec3 cross(const vec3 &u, const vec3 &v) {
    return vec3(u.e[1] * v.e[2] - u.e[2] * v.e[1],
                u.e[2] * v.e[0] - u.e[0] * v.e[2],
                u.e[0] * v.e[1] - u.e[1] * v.e[0]);
}

inline vec3 unit_vector(vec3 v) {
    return v / (v.length() + 0.001f);
}

inline vec3 random_vector(){
    return vec3(random(), random(), random());
}

inline vec3 random_vector(float min_num, float max_num){
    return vec3(
        random(min_num, max_num),
        random(min_num, max_num),
        random(min_num, max_num)
    );
}

inline vec3 random_in_unit_sphere(){
    while (true){
        vec3 pos = random_vector(-1, 1);
        if (pos.length_squared() < 1){
            return pos;
        }
    }
}

inline vec3 random_unit_vector(){
    return unit_vector(random_in_unit_sphere());
}

inline vec3 reflect(const vec3& v, const vec3& normal){
    return v - 2 * dot(v, normal) * normal;
}

inline vec3 refract(const vec3& v, const vec3& normal, float eta_rate){
    float cos_theta = fmin(dot(-v, normal), 1.0);
    vec3 v_perp = eta_rate * (v + cos_theta * normal);
    vec3 v_parallel = -sqrt(fabs(1 - dot(v_perp, v_perp))) * normal;
    return v_perp + v_parallel;
}

#endif
