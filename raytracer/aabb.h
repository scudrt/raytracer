#ifndef AABB_H
#define AABB_H

#include "rtutil.h"

class aabb {
    public:
        aabb(){}
        aabb(const point3 min_p, const point3& max_p):
			min_pos(min_p), max_pos(max_p), mid_pos(0.5 * (min_pos + max_pos)){}

        point3 min() const {return min_pos;}
        point3 max() const {return max_pos;}
		point3 mid() const {return mid_pos;}

        // return if ray hit the aabb
        inline bool hit(const ray& r, float tmin, float tmax) const {
            float start, end, temp;
            for (int i=0; i<3; ++i){
                temp = 1.0 / r.direction()[i];
                start = (min_pos[i] - r.origin()[i]) * temp;
                end = (max_pos[i] - r.origin()[i]) * temp;
                if (temp < 0) std::swap(start, end);
                tmin = start > tmin ? start : tmin;
                tmax = end < tmax ? end : tmax;
                if (tmin >= tmax) return false;
            }
            return true;
        }
    private:
		point3 min_pos, max_pos, mid_pos;
};

aabb combined_box(const aabb& box_a, const aabb& box_b){
    point3 minpos = point3(
        fmin(box_a.min().x(), box_b.min().x()),
        fmin(box_a.min().y(), box_b.min().y()),
        fmin(box_a.min().z(), box_b.min().z())
    ), maxpos = point3(
        fmax(box_a.max().x(), box_b.max().x()),
        fmax(box_a.max().y(), box_b.max().y()),
        fmax(box_a.max().z(), box_b.max().z())
    );
    return aabb(minpos, maxpos);
}

#endif
