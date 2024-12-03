#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtutil.h"
#include "rtutil_stb_image.h"

class texture{
    public:
        virtual color sample(float u, float v) const = 0;
    private:
};

// texture with single color
class solid_texture: public texture{
    public:
        solid_texture(){}
        solid_texture(float r, float g, float b): solid_texture(color(r, g, b)){}
        solid_texture(const color& texture_color): texture_color(texture_color){}

        virtual inline color sample(float u, float v) const override {
            return texture_color;
        }

    private:
        color texture_color;
};

class image_texture: public texture{
    public:
        static const int pixel_size = 3;

        image_texture(){}
        image_texture(const char* filename){
            int temp = pixel_size;
            this->data = stbi_load(filename, &width, &height, &temp, temp);
            if (!data){
                std::cerr << "image not found: " << filename << std::endl;
                return;
            }

            bytes_per_line = pixel_size * width;
        }

        virtual ~image_texture(){ delete[] data; }

        virtual color sample(float u, float v) const override {
            if (data == nullptr) return color(0, 0, 0);
			u = u * 3.4f - 2.075f;
			v = v * 2.2f - 0.9f;
            u = clamp(u, 0, 1);
            v = 1.0 - clamp(v, 0, 1); // image starts from top left corner

            int col = (int)clamp(u * width, 0, width - 1),
                row = (int)clamp(v * height, 0, height - 1);
            unsigned char* value_ptr = &data[row * bytes_per_line + col * pixel_size];
            const float temp = 1.0 / 255.0;
            return color(value_ptr[0], value_ptr[1], value_ptr[2]) * temp;
        }
    private:
        unsigned char* data;
        int width, height, bytes_per_line;
};

#endif
