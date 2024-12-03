#ifndef PDF_H
#define PDF_H

#include <cmath>

#include "hittable.h"
#include "rtutil.h"
#include "vec3.h"
#include "onb.h"

class pdf {
public:
	virtual float value(const vec3& dir) const= 0;
	virtual vec3 generate() const = 0;
private:
	;
};

class cosine_pdf : public pdf {
public:
	cosine_pdf(const vec3& normal) : normal(unit_vector(normal)) {}

	virtual float value(const vec3& dir) const {
		return fmax(0.0, dot(unit_vector(dir), normal)) * pi_inv;
	}

	virtual vec3 generate() const {
		float phi = 2 * pi * random();
		float costheta = random(), sintheta = sqrt(1 - costheta);
		costheta = sqrt(costheta);

		onb face;
		face.build_from(normal);

		return face.local(vec3(
			sintheta * cos(phi),
			sintheta * sin(phi),
			costheta
		));
	}
private:
	vec3 normal;
};

class hemi_pdf : public pdf {
public:
	hemi_pdf(const vec3& normal) : normal(normal) {}

	virtual float value(const vec3& dir) const {
		return dot(normal, dir) > 0 ? 0.5 * pi_inv : 0.0;
	}

	virtual vec3 generate() const {
		float phi = 2 * pi * random();
		float costheta = random(), sintheta = sqrt(1 - costheta * costheta);

		onb face;
		face.build_from(normal);

		return face.local(vec3(
			sintheta * cos(phi),
			sintheta * sin(phi),
			costheta
		));
	}
private:
	vec3 normal;
};

class hittable_pdf : public pdf {
public:
	hittable_pdf(const vec3& from, shared_ptr<hittable> target) : from(from), target(target) {}

	virtual float value(const vec3& dir) const {
		return target->pdf_value(from, dir);
	}

	virtual vec3 generate() const {
		return target->random_to_face(from);
	}
private:
	vec3 from;
	shared_ptr<hittable> target;
};

class mixture_pdf : public pdf {
public:
	mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) : p0(p0), p1(p1) {}

	virtual float value(const vec3& dir) const override {
		return 0.5 * (p0->value(dir) + p1->value(dir));
	}

	virtual inline vec3 generate() const override {
		return random() < 0.5f ? p0->generate() : p1->generate();
	}
private:
	shared_ptr<pdf> p0, p1;
};

#endif
