#ifndef CONST_MEDIUM_H
#define CONST_MEDIUM_H

#include <iostream>

#include "rtutil.h"
#include "hittable.h"
#include "texture.h"
#include "material.h"

class const_medium : public hittable {
public:
	const_medium(shared_ptr<hittable> thing, float density, const color& c)
		: boundary(thing), inner_mat(make_shared<isotropic>(c)), neg_inv_density(-1.0 / (density < 0.0001 ? 0.0001 : density)) {
	}

	const_medium(shared_ptr<hittable> thing, float density, shared_ptr<texture> tex)
		: boundary(thing), inner_mat(make_shared<isotropic>(tex)), neg_inv_density(-1.0 / (density < 0.0001 ? 0.0001 : density)) {
	}

	virtual bool hit(const ray& r, float mint, float maxt, hit_record& hitinfo) const {
		hit_record hit0, hit1;
		if (!boundary->hit(r, -infinity, infinity, hit0)) return false;
		if (!boundary->hit(r, hit0.t + 0.0001, infinity, hit1)) return false;

		mint = fmax(mint, 0);
		hit0.t = fmax(hit0.t, mint);
		hit1.t = fmin(hit1.t, maxt);
		if (hit0.t >= hit1.t) return false;

		float ray_length = r.direction().length();
		float inner_length = (hit1.t - hit0.t) * ray_length;
		double scatter_distance = log(random()) * neg_inv_density;
		if (scatter_distance > inner_length) return false;

		hitinfo.t = hit0.t + scatter_distance / ray_length;
		hitinfo.pos = r.at(hitinfo.t);
		hitinfo.mat_ptr = inner_mat;
		hitinfo.u = hit0.u;
		hitinfo.v = hit0.v;

		// optional
		hitinfo.is_front_face = true;
		hitinfo.normal = vec3(0, 1, 0);

		return true;
	}

	virtual bool bounding_box(aabb& out_box) const {
		return boundary->bounding_box(out_box);
	}

private:
	shared_ptr<hittable> boundary;
	shared_ptr<material> inner_mat; //material of the inner matter
	float neg_inv_density;
};


#endif
