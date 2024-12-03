#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtutil.h"
#include "aabb.h"

class material;
class hittable;

struct hit_record {
    point3 pos;
    vec3 normal;
    float t, u, v;
    shared_ptr<material> mat_ptr = nullptr;
	shared_ptr<hittable> object = nullptr;
    bool is_front_face;

    inline void set_face_normal(const ray& r, vec3 outward_normal){
        is_front_face = dot(r.direction(), outward_normal) < 0;
        normal = unit_vector(is_front_face ? outward_normal : -outward_normal);
    }
};

class hittable {
    public:
        virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const = 0;
        virtual bool bounding_box(aabb& box) const = 0;

		virtual vec3 random_to_face(const vec3& pos_to_face) const {
			// meaningless for some hittable objects
			return vec3(0, 0, 0);
		}

		virtual float pdf_value(const vec3& from, const vec3& dir) const {
			return 0.0f;
		}
};

#endif
