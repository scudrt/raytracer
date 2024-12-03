#ifndef RTUTIL_H
#define RTUTIL_H

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;

// Constants

const float infinity = std::numeric_limits<float>::infinity();
const float pi = 3.1415926535897932385;
const float pi_inv = 1.0 / pi;

// Utility Functions

inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

//return random value in [0, 1)
inline float random(){
    return rand() / (RAND_MAX + 1.0f);
}

inline double random_double() {
	return (double)1.0 * (double)rand() / (RAND_MAX + 1.0);
}

inline float random(float min_num, float max_num){
    return min_num + (max_num - min_num) * random();
}

inline float clamp(float x, float min_num, float max_num){
    if (x < min_num) return min_num;
    return x > max_num ? max_num : x;
}

// Common Headers

#include "ray.h"
#include "vec3.h"

#endif
