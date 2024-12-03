#ifndef ONB_H
#define ONB_H

#include "vec3.h"

// orthonormal basis
class onb {
public:
	onb() {}

	inline vec3 operator[] (int index) const { return axis[index]; }

	vec3 u() const { return axis[0]; }
	vec3 v() const { return axis[1]; }
	vec3 w() const { return axis[2]; }

	vec3 local(float a, float b, float c) {
		return a * axis[0] + b * axis[1] + c * axis[2];
	}

	vec3 local(const vec3& a) {
		return a.x() * u() + a.y() * v() + a.z() * w();
	}

	void build_from(const vec3& normal);

private:
	vec3 axis[3];
};

void onb::build_from(const vec3& normal) {
	axis[2] = unit_vector(normal); //forward
	vec3 a = axis[2].x() > 0.9 ? vec3(0, 1, 0) : vec3(1, 0, 0);
	axis[1] = unit_vector(cross(axis[2], a)); //up
	axis[0] = cross(axis[2], axis[1]); //right
}

#endif
