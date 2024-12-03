#ifndef BVH_H
#define BVH_H

#include <algorithm>

#include "rtutil.h"
#include "hittable.h"
#include "hittable_list.h"

inline bool comparator(const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis) {
    aabb boxa, boxb;
    a->bounding_box(boxa);
    b->bounding_box(boxb);
    return boxa.min().e[axis] < boxb.min().e[axis];
}

bool comparator_x(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return comparator(a, b, 0);
}

bool comparator_y(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return comparator(a, b, 1);
}

bool comparator_z(const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return comparator(a, b, 2);
}

class bvh_node: public hittable{
    public:
        bvh_node(){}
        bvh_node(hittable_list& hittables): bvh_node(hittables.objects, 0, hittables.objects.size()) {}
		bvh_node(std::vector<shared_ptr<hittable> >& objects, int start, int end);

        virtual bool hit(const ray& r, float min_t, float max_t, hit_record& hitinfo) const override;

        virtual bool bounding_box(aabb& out_box) const override;
    private:
        shared_ptr<hittable> left, right;
		int start, end;
        aabb box;
		std::vector<shared_ptr<hittable> > *objects;
};

// constructing bvh tree
bvh_node::bvh_node(std::vector<shared_ptr<hittable> >& objects, int start, int end): objects(&objects) {
	this->start = start;
	this->end = end;
    int axis = (int)(random() * 3.0),
        size = end - start;

    auto compare = axis == 2 ? comparator_z : (axis == 1 ? comparator_y : comparator_x);
    
    if (size == 1){
        left = right = objects[start];
    } else if (size == 2){
        if (compare(objects[start], objects[start + 1])){
            left = objects[start];
            right = objects[start + 1];
        }else{
            left = objects[start + 1];
            right = objects[start];
        }
    } else { // sort and divide
        std::sort(objects.begin() + start, objects.begin() + end, compare);
        
        int mid = (start + end) >> 1;

        left = make_shared<bvh_node>(objects, start, mid);
        right = make_shared<bvh_node>(objects, mid, end);
    }

    aabb boxl, boxr;
    left->bounding_box(boxl);
    right->bounding_box(boxr);
    box = combined_box(boxl, boxr);
}

bool bvh_node::bounding_box(aabb& out_box) const {
    out_box = this->box;
    return true;
}

// return the nearest ray hit info
bool bvh_node::hit(const ray& r, float min_t, float max_t, hit_record& hitinfo) const {
    if (!box.hit(r, min_t, max_t)) return false;

    bool hit_left = left->hit(r, min_t, max_t, hitinfo),
        hit_right = right->hit(r, min_t, (hit_left ? hitinfo.t : max_t), hitinfo);
    
    return hit_left || hit_right;
}

#endif
