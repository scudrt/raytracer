#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <algorithm>

#include "hittable.h"
#include "ver.h"
#include "onb.h"

class triangle : public hittable {
public:
	triangle() {}
	triangle(const ver& a, const ver& b, const ver& c, shared_ptr<material> mat)
		: v { a, b, c }, mat_ptr(mat) {
		__area = 0.5 * cross(v[0].pos - v[1].pos, v[0].pos - v[2].pos).length();
	};

	virtual bool hit(
		const ray& r, float t_min, float t_max, hit_record& hit) const override;

	bool hit(const ray& r, float t_min, float t_max) const;

	virtual bool bounding_box(aabb& box) const override;

	// return random point on triangle of indicated direction
	virtual vec3 random_to_face(const vec3& pos_to_face) const override {
		// called when it is a light source
		float u = random(),
			vv = random();
		bool needFlip = vv > 1 - u;
		u = needFlip ? 1 - u : u;
		vv = needFlip ? 1 - vv : vv;
		vec3 pos = u * v[0].pos + vv * v[1].pos + (1 - u - vv) * v[2].pos;
		return unit_vector(pos - pos_to_face);
	}

	virtual float pdf_value(const vec3& from, const vec3& dir) const override {
		if (!hit(ray(from, dir), 0.001, infinity)) return 0;

		float cosine = -dot(dir, v[0].normal);
		return cosine > 0 ? 1.0 / (__area * cosine) : 0;
	}

public:
	ver v[3];
	shared_ptr<material> mat_ptr;
	float __area;
};

bool triangle::hit(const ray& r, float t_min, float t_max, hit_record& hit) const {
	vec3 E1 = v[1].pos - v[0].pos,
		E2 = v[2].pos - v[0].pos;
	vec3 M = cross(r.dir, E2);
	double det = dot(M, E1);
	if (det == 0) return false; // parallel or overlap
	det = 1.0 / det; // inverse for fast computation

	vec3 T = r.orig - v[0].pos;
	vec3 K = cross(T, E1);

	float t = dot(K, E2) * det;
	if (t < t_min || t > t_max) return false;

	float u = dot(M, T) * det,
		vv = dot(K, r.dir) * det;
	if (u < 0 || vv < 0 || (u + vv > 1.0)) return false;

	float w = 1.0 - u - vv;
	hit.t = t;
	hit.pos = r.at(t);
	hit.mat_ptr = mat_ptr;
	hit.set_face_normal(r, v[1].normal);
	hit.u = w * v[0].u + u * v[1].u + vv * v[2].u;
	hit.v = w * v[0].v + u * v[1].v + vv * v[2].v;
	return true;
}

bool triangle::hit(const ray& r, float t_min, float t_max) const {
	vec3 E1 = v[1].pos - v[0].pos,
		E2 = v[2].pos - v[0].pos;
	vec3 M = cross(r.dir, E2);
	double det = dot(M, E1);
	if (det <= 1e-7) return false; // parallel or overlap
	det = 1.0 / det; // inverse for fast computation

	vec3 T = r.orig - v[0].pos;
	vec3 K = cross(T, E1);

	float t = dot(K, E2) * det;
	if (t < t_min || t > t_max) return false;

	float u = dot(M, T) * det,
		vv = dot(K, r.dir) * det;
	if (u < 0 || vv < 0 || (u + vv > 1.0)) return false;
	return true;
}

bool triangle::bounding_box(aabb& box) const {
	vec3 minn = v[0].pos, maxx = v[0].pos;
	for (int i = 0; i < 3; ++i) {
		minn[i] = std::min(minn[i], v[1].pos[i]);
		maxx[i] = std::max(maxx[i], v[1].pos[i]);
		minn[i] = std::min(minn[i], v[2].pos[i]);
		maxx[i] = std::max(maxx[i], v[2].pos[i]);
	}
	box = aabb(minn, maxx);
	return true;
}

#endif
