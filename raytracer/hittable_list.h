#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "aabb.h"
#include "pdf.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable {
    public:
        hittable_list() {}
        hittable_list(shared_ptr<hittable> object) { add(object); }

        void clear() { objects.clear(); }
        void add(shared_ptr<hittable> object) {
			objects.push_back(object);
			size_inv = 1.0 / objects.size();
		}
		void add_list(shared_ptr<hittable_list> list) {
			objects.insert(objects.end(), list->objects.begin(), list->objects.end());
			size_inv = 1.0 / objects.size();
		}

        virtual bool hit(
            const ray& r, float t_min, float t_max, hit_record& rec) const override;

        virtual bool bounding_box(aabb& box) const override;

		inline int size() const {
			return objects.size();
		}

		const shared_ptr<hittable> random_object() const {
			return objects[(int)random(0, objects.size())];
		}

		virtual vec3 random_to_face(const vec3& from) const;

		virtual float pdf_value(const vec3& from, const vec3& dir) const;

    public:
        std::vector<shared_ptr<hittable>> objects;

		float size_inv;
};

bool hittable_list::hit(const ray& r, float t_min, float t_max, hit_record& rec) const {
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = t_max;

    for (const auto& object : objects) {
        if (object->hit(r, t_min, closest_so_far, temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
    }

    return hit_anything;
}

bool hittable_list::bounding_box(aabb& box) const {
    if (objects.empty()) return false;

    aabb temp_box;
    bool is_first = true;
    for (auto object: objects){
        if (!object->bounding_box(temp_box)){ return false;}
        box = is_first ? temp_box : combined_box(temp_box, box);
    }
    return true;
}

vec3 hittable_list::random_to_face(const vec3& from) const {
	return random_object()->random_to_face(from);
}

float hittable_list::pdf_value(const vec3& from, const vec3& dir) const {
	float sum = 0;
	for (auto object: objects) {
		sum += object->pdf_value(from, dir);
	}
	return sum * size_inv;
}

#endif
