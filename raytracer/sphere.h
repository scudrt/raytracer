#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"
#include "ray.h"
#include "onb.h"

class sphere : public hittable {
    public:
        sphere() {}
        sphere(point3 pos, float r, shared_ptr<material> mat) : center(pos), radius(r),
			radius_square(r * r), mat_ptr(mat){};

		virtual bool hit(
			const ray& r, float t_min, float t_max, hit_record& hit) const override;

		bool hit(const ray& r, float t_min, float t_max) const;

        virtual bool bounding_box(aabb& box) const override;

        // return sphere uv in a unit sphere
        static void get_sphere_uv(const point3& pos, float& u, float& v){
            u = 0.5 * (atan2(-pos.z(), pos.x()) + pi) * pi_inv;
            v = acos(-pos.y()) * pi_inv;
        }

		// return random point on sphere of indicated direction
		virtual vec3 random_to_face(const vec3& pos_to_face) const override {
			float cosine = sqrt(1 - radius_square / (pos_to_face - center).length_squared());

			float phi = random(),
				z = 1 - random() * (1 - cosine),
				k = sqrt(1 - z * z),
				x = k * cos(phi),
				y = k * sin(phi);

			onb face_to;
			face_to.build_from(center - pos_to_face);
			return face_to.local(x, y, z);
		}

		virtual float pdf_value(const vec3& from, const vec3& dir) const override {
			if (!hit(ray(from, dir), 0.001, infinity)) return 0.0;

			float cosine = 1 - radius_square / (from - center).length_squared();
			if (cosine > 0) return pi_inv / (2 * (1.0 - sqrt(cosine)));

			return 0;
		}

    public:
        point3 center;
		float radius, radius_square;
        shared_ptr<material> mat_ptr;
};

bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& hit) const {
	vec3 to_center = this->center - r.orig;
	float dist2 = to_center.length_squared();
	bool is_inside = dist2 <= radius_square;
	float dot_map = dot(to_center, r.dir);
	if (dot_map < 0 && !is_inside) return false; // not towards the sphere

	float m2 = dist2 - dot_map * dot_map;
	if (m2 > radius_square) return false;

	float q = sqrt(radius_square - m2);
	hit.t = dot_map + (is_inside ? q : -q);
	if (hit.t < t_min || hit.t > t_max) return false;

	hit.pos = r.at(hit.t);
	hit.mat_ptr = this->mat_ptr;
	hit.is_front_face = !is_inside;
	vec3 outward_normal = (hit.pos - center) / radius;
	hit.normal = is_inside ? -outward_normal : outward_normal;
	get_sphere_uv(outward_normal, hit.u, hit.v);

	return true;
	/*
	vec3 oc = r.origin() - center;
	auto a = r.direction().length_squared();
	auto half_b = dot(oc, r.direction());
	auto c = oc.length_squared() - radius_square;

	auto discriminant = half_b * half_b - a * c;
	if (discriminant < 0) return false;
	float sqrtd = sqrt(discriminant);

	// Find the nearest root that lies in the acceptable range.
	float root = (-half_b - sqrtd) / a;
	if (root < t_min || t_max < root) {
		root = (-half_b + sqrtd) / a;
		if (root < t_min || t_max < root)
			return false;
	}

	hit.t = root;
	hit.pos = r.at(root);
	hit.mat_ptr = this->mat_ptr;
	vec3 outward_normal = (hit.pos - center) / radius;
	hit.set_face_normal(r, outward_normal);
	get_sphere_uv(outward_normal, hit.u, hit.v);

	return true;*/
}

//fast version of sphere::hit
bool sphere::hit(const ray& r, float t_min, float t_max) const {
	vec3 to_center = this->center - r.orig;
	float dist2 = to_center.length_squared();
	if (dist2 <= radius_square) return true; // inside the sphere

	float dot_map = dot(to_center, r.dir);
	if (dot_map < 0) return false; // not towards the sphere

	float m2 = dist2 - dot_map * dot_map;
	if (m2 > radius_square) return false;

	float t = dot_map - sqrt(radius_square - m2);
	if (t < t_min || t > t_max) return false;
	return true;
}

bool sphere::bounding_box(aabb& box) const {
    box = aabb(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius));
    return true;
}

#endif
