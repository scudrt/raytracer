/*
ģ������
ë��
Ƥ��
�Ʋ�
ˮ��
����
*/
#include <iostream>
#include <iomanip>
#include <future>
#include <chrono>
#include <atomic>
#include <ctime>

#include "rtutil.h"

#include "bvh.h"
#include "pdf.h"
#include "color.h"
#include "model.h"
#include "camera.h"
#include "sphere.h"
#include "material.h"
#include "const_medium.h"

// 4:	7465522.197
// 5:	7683831.344
// 10:	11322350.777
// 16:	11561140.811
// 31:	13032896.907

// 1281.12s 1920 * 1080 * 1024 / 4 threads
//screen
constexpr float aspect = 16.0 / 9.0;
const int image_width = 800;
constexpr int image_height = image_width / aspect;

const int samples_per_pixel = 512;

const int max_ray_depth = 8;

color screen[image_height][image_width];

// multi-thread
volatile float pixel_u, pixel_v;
std::atomic_int64_t ray_scattered(0);

color* colors;
std::atomic_int remain_lines(image_height); // 0 ready, 1 pause / finished, 2 exit

hittable_list object_list;
bvh_node scene;
shared_ptr<hittable_list> lights = make_shared<hittable_list>();

// Camera
camera cam(point3(420, 800, -470), point3(0, 100, 0), vec3(0, 1, 0), 45.0, aspect);

//return sky color
color blue_sky(const ray& r){
    auto t = 0.5 * (r.dir.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.4, 0.6, 1.0);
}

inline color night(const ray& r) {
	return vec3(0.01, 0.01, 0.01) * 0;
	auto t = 0.5 * (r.dir.y() + 1.0);
	return (1.0 - t) * color(0.04, 0.03, 0.06) + t * color(0.01, 0.01, 0.02);
}

color __ray_color(const ray& r, int depth){
	if (!depth) return vec3(0, 0, 0);
	++ray_scattered;

	hit_record hitinfo;
	if (!scene.hit(r, 0.1, infinity, hitinfo)) return blue_sky(r);

	scatter_record sinfo;
	// sample the indirectional light
	if (!hitinfo.mat_ptr->scatter(r, hitinfo, sinfo)) return hitinfo.mat_ptr->emit(hitinfo);

	// return sinfo.attenuation * cosine * ray_color(sinfo.r, depth - 1);
	
	if (sinfo.is_specular) {
		return sinfo.attenuation * __ray_color(sinfo.r, depth - 1);
	}

	auto light = lights->random_object();
	auto light_pdf = make_shared<hittable_pdf>(hitinfo.pos, lights);
	auto mix_pdf = make_shared<mixture_pdf>(sinfo.scatter_pdf, light_pdf);

	vec3 dir = mix_pdf->generate();
	float pdf_rate = sinfo.scatter_pdf->value(dir) / mix_pdf->value(dir);
	return sinfo.attenuation * __ray_color(ray(hitinfo.pos, dir), depth - 1) * pdf_rate;
}

color ray_color(ray& r, int depth) {
	color ret(1, 1, 1);
	hit_record hitinfo;
	scatter_record sinfo;
	while (depth > 0) {
		--depth;
		++ray_scattered;

		if (!scene.hit(r, 0.1f, infinity, hitinfo)) return ret * blue_sky(r);
		// sample the indirectional light
		if (!hitinfo.mat_ptr->scatter(r, hitinfo, sinfo)) {
			// color emitColor = hitinfo.mat_ptr->emit(hitinfo);
			auto emit = hitinfo.mat_ptr->emit(hitinfo);
			return ret * emit;
		}

		if (sinfo.is_specular) {
			ret *= sinfo.attenuation;
			r = sinfo.r;
			continue;
		}

		// auto light = lights->random_object();
		auto light_pdf = make_shared<hittable_pdf>(hitinfo.pos, lights);
		auto mix_pdf = make_shared<mixture_pdf>(sinfo.scatter_pdf, light_pdf);

		vec3 dir = mix_pdf->generate();
		ret *= sinfo.attenuation * sinfo.scatter_pdf->value(dir) / mix_pdf->value(dir);
		r = ray(hitinfo.pos, dir);
	}
	return vec3(0, 0, 0);
}

// preventing the bad pixel error
inline void fix_color(vec3& c) {
	const float color_value_limit = 2.0;
	auto r = c.x();
	auto g = c.y();
	auto b = c.z();

	if (r != r) r = 0; r = fmin(r, color_value_limit);
	if (g != g) g = 0; g = fmin(g, color_value_limit);
	if (b != b) b = 0; b = fmin(b, color_value_limit);

	c = vec3(r, g, b);
}

inline color ray_color_async(int tid, int step) {
	srand(tid + 114514);

	color *c;
	color temp;
	float inv_w = 1.0 / (image_width - 1), inv_h = 1.0 / (image_height - 1);
	for (int i = tid;i < image_height;i += step) {
		c = screen[i];
		for (int j = 0;j < image_width;++j) {
			*c = color(0, 0, 0);
			for (int k = 0;k < samples_per_pixel;++k) {
				ray r = cam.get_ray(
					(j + random()) * inv_w,
					(i + random()) * inv_h
				);
				temp = ray_color(r, max_ray_depth);
				fix_color(temp);
				(*c) += temp;
			}
			++c;
		}
		--remain_lines;
		std::cerr << "\rRemaining lines: " << remain_lines << "        " << std::flush;
	}
	return vec3(0, 0, 0);
}

void ray_color_multithread() {
	int thread_num = std::thread::hardware_concurrency() - 1;
	std::cerr << "hardware concurrency: " << thread_num << std::endl;

	std::vector<std::future<color>> ts(thread_num);
	for (int j = 0;j < thread_num;++j) {
		ts[j] = std::async(ray_color_async, j, thread_num);
	}
	for (int i = 0; i < thread_num; ++i) ts[i].get();

	for (int i = image_height - 1;i >= 0;--i) {
		for (int j = 0;j < image_width;++j) {
			write_color(std::cout, screen[i][j], samples_per_pixel);
		}
	}
}

void random_generate(int square_size){
	auto mat_ground = make_shared<lambertian>(color(0.6, 0.6, 1.0));
	auto mat_earth = make_shared<light>(make_shared<image_texture>("pipboy.png"), 1);
	auto mat_metal = make_shared<metal>(color(0.7, 0.6, 0.5), 0.04);
	auto mat_light = make_shared<light>(color(1, 1, 1), 0.5);
	auto mat_glass = make_shared<dielectric>(1.1);

	object_list.add(make_shared<sphere>(point3(0.0, -50000.5, -1.0), 50000.0, mat_ground));
	// auto alight = make_shared<sphere>(point3(-0.5, 0.5, 0), 1.0, mat_glass);
	// lights->add(alight);
	// object_list.add(alight);
	
	auto l = make_shared<sphere>(vec3(0, 4000, 0), 1600, mat_light);
	object_list.add(l);
	lights->add(l);
	/*
	auto ball = make_shared<sphere>(vec3(0, 100, 0), 100, mat_glass);
	object_list.add(ball);
	auto subsurface = make_shared<const_medium>(ball, 0.25, color(0.9, 0.9, 0.8));
	object_list.add(subsurface);
	auto inner_light = make_shared<sphere>(vec3(0, 100, 0), 98, mat_earth);
	object_list.add(inner_light);
	lights->add(inner_light);
	*/
	return;
	
    int mat_index;
    square_size /= 2;
    for (int x=-square_size * 200; x<square_size * 200; x += 200){
        for (int z=-square_size * 200; z<square_size * 200; z += 200){
            point3 pos = vec3(x + random() * 120, 40, z + random() * 120);
            if (pos.length_squared() >= 10000){
                mat_index = int(random() * 100);
                if (mat_index <= 30){ // lambertian
                    auto mat = make_shared<lambertian>(random_in_unit_sphere() * 0.5 + 0.5);
                    object_list.add(make_shared<sphere>(pos, 40, mat));
                } else if (mat_index <= 60){ // metal
                    auto mat = make_shared<metal>(random_in_unit_sphere() * 0.5 + 0.5, random() * 0.1);
					object_list.add(make_shared<sphere>(pos, 40, mat));
                } else if (mat_index <= 95){ // glass
                    auto mat = make_shared<dielectric>(random() + 1.2);
					object_list.add(make_shared<sphere>(pos, 40, mat));
                } else { // light
					continue;
                    color light_color = vec3(0.5, 0.5, 0.5) + random_in_unit_sphere() * 0.5 + 0.5;
                    auto mat = make_shared<light>(light_color, 1);
					auto light = make_shared<sphere>(pos, 40, mat);
					object_list.add(light);
                    lights->add(light);
                }
            }
        }
    }
}

int main() {
	int seed = 77777; // 114514
	std::cerr << "seed: " << seed << std::endl;

    // Scene
    random_generate(16);

	/*
	shared_ptr<model> cat = make_shared<model>(
		"cats_obj/cats_obj.obj",
		vec3(0, 0.5, 0),
		1,
		make_shared<light>(color(1, 1, 0.8), 1)
		/*
		"skull/skull.obj",
		vec3(100, 0.5, 0),
		50.0f,
		make_shared<metal>(color(1, 1, 1), 0.05)
		*//*
	);
	object_list.add_list(cat);
	auto cat_outer = make_shared<model>(
		"cats_obj/cats_obj.obj",
		vec3(0, 0.5f, 0),
		1.01f,
		make_shared<lambertian>(color())
	);
	auto cat_bvh = make_shared<bvh_node>(cat_outer->objects, 0, cat_outer->objects.size());
	object_list.add(make_shared<const_medium>(cat_bvh, 0.0625, color(1, 1, 1) * 0.075));
	std::cerr << "num of scene objects: " << object_list.size() << std::endl;
	*/

	auto mat0 = make_shared<dielectric>(1.6);
	auto mat1 = make_shared<metal>(color(0.75, 1, 1), 0.01);
	auto mat2 = make_shared<lambertian>(color(0.8, 0.8, 0.8));
	object_list.add(make_shared<sphere>(vec3(-300, 150, -100), 150, mat0));
	object_list.add(make_shared<sphere>(vec3(100, 150, 300), 150, mat1));
	object_list.add(make_shared<sphere>(vec3(350, 150, -375), 150, mat2));
    // Rendering
	// head data of image
    std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	scene = bvh_node(object_list);

	time_t start_time = clock();
	ray_color_multithread();
	auto time_spent = clock() - start_time;

    std::cerr << "\ntime spent: " << time_spent << " ms\n";
	std::cerr << "ray scattered: " << ray_scattered << std::endl;
	std::cerr << "average:" << ray_scattered / float(image_height * image_width * samples_per_pixel) << " rays per sample" << std::endl;
	std::cerr << std::fixed << std::setprecision(3) << "speed: " << 1000.0 * ray_scattered / time_spent << " rays per second." << std::endl;

	system("pause");
}
