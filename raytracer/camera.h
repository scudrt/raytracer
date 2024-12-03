#ifndef CAMERA_H
#define CAMERA_H

#include "rtutil.h"

class camera {
    public:
        camera(
            point3 pos, vec3 lookat, vec3 up,
            float fov, float aspect) {
            fov = degrees_to_radians(fov);
            auto focal_length = 1.0;
            auto viewport_height = focal_length * tan(fov / 2.0) * 2.0;
            auto viewport_width = aspect * viewport_height;

            vec3 w = unit_vector(lookat - pos);
            vec3 u = unit_vector(cross(w, up));
            vec3 v = cross(u, w);

            origin = pos;
            horizontal = viewport_width * u;
            vertical = viewport_height * v;
            lower_left_corner = origin - horizontal / 2 - vertical / 2 + w;
        }

        ray get_ray(float u, float v) const {
            return ray(origin, lower_left_corner + u*horizontal + v*vertical - origin);
        }

    private:
        point3 origin;
        point3 lower_left_corner;
        vec3 horizontal;
        vec3 vertical;
};
#endif
