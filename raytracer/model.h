#pragma once

#include <map>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include "hittable_list.h"
#include "triangle.h"
#include "bvh.h"
#include "vec3.h"
#include "ver.h"
#include "onb.h"

class model : public hittable_list {
public:
	model() {}
	model(const std::string& obj_file, vec3 pos, float scale, shared_ptr<material> mat){
		loadModel(obj_file, pos, scale, mat);
	}

	void loadModel(const std::string& file, vec3 offset, float scale, shared_ptr<material> mat);
public:
	point3 pos;
	float scale;
};

vec3 generateNormal(const vec3& a, const vec3& b, const vec3& c) {
	vec3 ret = cross(c - b, b - a);
	return ret / ret.length();
}

/* forget it now
//return pairs of texture name and texture index
std::map<std::string, int>* _loadMTL(const std::string& dir, const std::string& path) {
	std::ifstream in(dir + path);
	if (in.is_open() == false) {
		in.close();
		return nullptr;
	}
	//pairs of texture path and texture index
	std::map<std::string, int>* ret = new std::map<std::string, int>();
	std::string name, temp; //std::string buffers
	char ch; //character buffer
	while (!in.eof()) {
		in >> temp;
		if (temp[0] == '#') { //skip the comments
			getline(in, temp);
			continue;
		}
		if (temp == "newmtl") { //beginning of mtl section
			in >> name;
		} else if (temp == "std::map_Kd") { //diffuse std::map
			in >> temp;
			if (ret->find(name + "std::map_Kd") == ret->end()) { //not been loaded yet
				UT3D::instance()->addTexture((dir + temp).c_str());
				(*ret)[name + "std::map_Kd"] = UT3D::instance()->textureBuffer.size();
			}
		} else if (temp == "std::map_Bump") { //normal std::map
			in >> temp;
			if (ret->find(name + "std::map_Bump") == ret->end()) { //not been loaded yet
				UT3D::instance()->addTexture((dir + temp).c_str());
				(*ret)[name + "std::map_Bump"] = UT3D::instance()->textureBuffer.size();
			}
		} else if (temp == "std::map_Ns") { //specular std::map
			in >> temp;
			if (ret->find(name + "std::map_Ns") == ret->end()) { //not been loaded yet
				UT3D::instance()->addTexture((dir + temp).c_str());
				(*ret)[name + "std::map_Ns"] = UT3D::instance()->textureBuffer.size();
			}
		} else if (temp == "") { //empty line, ignore it
			;
		} else { //not supported information
			getline(in, temp);
		}
	}

	in.close();
	return ret;
}
*/

//the code needs to be simplified
//only for .obj format
void model::loadModel(const std::string& file,
							   vec3 offset, float scale, shared_ptr<material> mat) {
	int index = file.find_last_of('/');
	index = std::max(index, (int)file.find_last_of('\\'));
	std::string dir = file.substr(0, index + 1); //direction of model

	std::ifstream in(file);

	std::vector<vec3> positions;

	//store texture coordinations
	std::vector<float> uvs;

	//array of normals
	std::vector<vec3> norms;

	std::vector<ver> vers;

	//for group to texture std::mapping
	std::map<std::string, int>* mtlmap = nullptr;
	std::string currentGroup;

	std::string buffer;
	int ia, ib, ic;
	float a, b, c;
	//.obj reading
	while (!in.eof()) {
		in >> buffer;
		if (buffer[0] == '#') { //comments
			getline(in, buffer);
			continue;
		}
		if (buffer[0] == 'v') {
			if (buffer == "v") { //vertex
				in >> a >> b >> c;
				getline(in, buffer); //might have vertex color
				//magic numbers, kind of model-to-world std::mapping
				positions.push_back(vec3(a, b, c) * scale + offset);
			} else if (buffer == "vn") { //world space normal
				in >> a >> b >> c;
				norms.push_back(vec3(a, b, c));
			} else if (buffer == "vt") { //texture coordinate
				in >> a >> b;
				uvs.push_back(a);
				uvs.push_back(b);
				getline(in, buffer); //skip the end
			}
		} else if (buffer == "f") { //Face
			getline(in, buffer);
			int len = buffer.size();
			bool hasNormal = true;

			vers.clear();
			//format: v[/vt/vn]
			for (int i = 0;i < len;++i) {
				//skip the empty first
				if (buffer[i] == ' ') continue;
				if (buffer[i] == '\n' || buffer[i] == '\0') break; //end of line

				ver v;
				// read position index
				ia = 0; // index of vertex a
				while (buffer[i] >= '0' && buffer[i] <= '9') {
					ia = ia * 10 + (buffer[i] - '0');
					++i;
				}
				v.pos = positions[ia - 1];

				// read uv coordination
				if (buffer[i] == '/') {
					++i;
					ib = 0;
					while (buffer[i] >= '0' && buffer[i] <= '9') {
						ib = ib * 10 + (buffer[i] - '0');
						++i;
					}
					v.u = uvs[ib * 2 - 2];
					v.v = uvs[ib * 2 - 1];

					//optional, read normal index
					if (buffer[i] == '/') {
						++i;
						ic = 0;
						while (buffer[i] >= '0' && buffer[i] <= '9') {
							ic = ic * 10 + (buffer[i] - '0');
							++i;
						}
						v.normal = norms[ic - 1];
					} else {
						hasNormal = false;
					}
				}
				// one vertex done
				vers.push_back(v);
			}
			// if (in.eof()) break;

			//generate normal for the face
			vec3 norm = generateNormal(vers[0].pos, vers[1].pos, vers[2].pos);
			for (int i = 0;i < vers.size();++i) {
				vers[i].normal = norm;
			}

			//cut faces into triangles, anti-clockwise
			for (int i = 1;i < vers.size() - 1;++i) {
				this->add(make_shared<triangle>(vers[0], vers[i], vers[i + 1], mat));
				/*
				//texture index
				int index = (*mtlmap)[currentGroup + "map_Kd"];
				vertices[ia].texIndex = index - 1;
				vertices[ib].texIndex = index - 1;
				vertices[ic].texIndex = index - 1;
				*/
			}
		} else if (buffer == "mtllib") { //Mtllib declaration
			in >> buffer;
			// mtlmap = _loadMTL(dir, buffer);
		} else if (buffer == "usemtl") { //Usemtl
			in >> buffer;
			currentGroup = buffer;
		} else if (buffer == "g") { //Group, skip it for now
			getline(in, buffer);
		} else if (buffer == "") { //empty line
			;
		} else { //s | others are skipped
			getline(in, buffer);
		}
	}

	in.close();
	if (mtlmap) delete mtlmap;
	uvs.clear();
	vers.clear();
	norms.clear();
	positions.clear();
	std::cerr << "num of model triangle: " << this->objects.size() << std::endl;
}
