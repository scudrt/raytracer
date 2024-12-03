#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtutil.h"
#include "texture.h"

struct hit_record;

struct scatter_record {
	ray r;
	color attenuation;
	shared_ptr<pdf> scatter_pdf;
	bool is_specular;
};

class material {
public:
    virtual bool scatter(const ray& r_in, const hit_record& rec, scatter_record& sinfo) const = 0;

	virtual vec3 emit(const hit_record& hitinfo) const {
		return vec3(0, 0, 0);
	};

	virtual float scatter_pdf(const ray& r, const hit_record& hitinfo, const scatter_record& sinfo) const {
		return sinfo.scatter_pdf->value(sinfo.r.dir);
	}
public:
	shared_ptr<texture> albedo;
};

class lambertian : public material {
public:
    lambertian(const color& a) : lambertian(make_shared<solid_texture>(a)) {}
    lambertian(shared_ptr<texture> texture){ albedo = texture;}

    virtual bool scatter(const ray& r, const hit_record& hitinfo, scatter_record& sinfo)
		const override {
		sinfo.scatter_pdf = make_shared<hemi_pdf>(hitinfo.normal);
		sinfo.is_specular = false;

        sinfo.r = ray(hitinfo.pos, sinfo.scatter_pdf->generate());
        sinfo.attenuation = albedo->sample(hitinfo.u, hitinfo.v);
        return true;
    }

	virtual float scatter_pdf(const ray& r, const hit_record& hitinfo, const scatter_record& sinfo)
		const override {
		return dot(hitinfo.normal, sinfo.r.direction()) > 0 ? pi_inv * 0.5 : 0.0;
	}

public:
};

class metal: public material{
public:
    metal(const color& a, float fuzzy): metal(make_shared<solid_texture>(a), fuzzy){}
    metal(shared_ptr<texture> albedo, float fuzzy): fuzzy(fuzzy < 1 ? fuzzy : 1){
		this->albedo = albedo;
	}
        
    virtual bool scatter(const ray& r, const hit_record& hitinfo, scatter_record& sinfo)
		const override {
        vec3 reflect_dir = reflect(r.direction(), hitinfo.normal);
        sinfo.r = ray(hitinfo.pos, reflect_dir + fuzzy * random_unit_vector());
		sinfo.is_specular = true;

        if (dot(sinfo.r.direction(), hitinfo.normal) > 0){
            sinfo.attenuation = albedo->sample(hitinfo.u, hitinfo.v);
            return true;
        }else{
            return false;
        }
    }

public:
    float fuzzy;
};

class dielectric: public material{
public:
    dielectric(float refractive_index): refractive_index(refractive_index) {}

    virtual bool scatter(const ray& r, const hit_record& hitinfo, scatter_record& sinfo)
		const override {
        sinfo.attenuation = color(1, 1, 1);

        float eta_rate = hitinfo.is_front_face ? (1.0 / refractive_index) : refractive_index;

        vec3 unit_ray_dir = unit_vector(r.direction());
        float cos_theta = fmin(dot(-unit_ray_dir, hitinfo.normal), 1.0);
        float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
        vec3 scatter_dir;
		bool need_reflect = sin_theta * eta_rate > 1.0 || reflectance(cos_theta, eta_rate) > random();
		scatter_dir = need_reflect ? reflect(unit_ray_dir, hitinfo.normal)
			: refract(unit_ray_dir, hitinfo.normal, eta_rate);
		/*
        if (){
            scatter_dir = reflect(unit_ray_dir, hitinfo.normal);
        }else{
            scatter_dir = refract(unit_ray_dir, hitinfo.normal, eta_rate);
        }*/
		sinfo.is_specular = true;
        sinfo.r = ray(hitinfo.pos, scatter_dir);

        return true;
    }
public:
    float refractive_index;
private:
    // Schlick's approximation for reflectance
    static float reflectance(float cosine, float eta_rate){
        auto r0 = (1 - eta_rate) / (1 + eta_rate);
        r0 *= r0;
        return r0 + (1- r0) * pow(1-cosine, 5);
    }
};

class light: public material{
public:
    light(color light_color, float intensity)
		: light(make_shared<solid_texture>(light_color), intensity){}

    light(shared_ptr<texture> light_color, float intensity)
        : intensity(intensity){ albedo = light_color; }

	virtual vec3 emit(const hit_record& hitinfo) const override {
		return intensity * albedo->sample(hitinfo.u, hitinfo.v);
	}

    virtual bool scatter(const ray& r, const hit_record& hitinfo, scatter_record& sinfo)
		const override {
		return false;
    }

public:
	float intensity;
};

class isotropic: public material {
public:
	isotropic(color albedo): isotropic(make_shared<solid_texture>(albedo)) {}
	isotropic(shared_ptr<texture> albedo) { this->albedo = albedo; }

	virtual bool scatter(const ray& r, const hit_record& hitinfo, scatter_record& sinfo)
		const override {
		sinfo.attenuation = albedo->sample(hitinfo.u, hitinfo.v);
		sinfo.r = ray(hitinfo.pos, random_in_unit_sphere());
		sinfo.is_specular = true;
		return true;
	}
};

#endif
