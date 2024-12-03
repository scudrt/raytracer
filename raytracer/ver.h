#pragma once

#include "vec3.h"

class ver {
	public:
	ver(){}
	ver(vec3 pos, vec3 normal, float u, float v)
		: pos(pos), normal(normal), u(u), v(v){}

	public:
	vec3 pos, normal;
	float u, v;
};
