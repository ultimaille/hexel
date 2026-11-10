#pragma once
#include "colormap.h"
#include <array>
#include <string>
#include <glad/gl.h>
#include <ultimaille/all.h>
#include <core/renderer.h>
using namespace UM;

bool no_gl_error();


struct PointRenderer: public Renderer{
	int radius_in_pixel=2;
	struct Vertex {
		std::array<float, 3> pos;
		float v;
		int visible;
		int primitive_id;
	};
	void generate_gui(std::string name);
	void init();
	void update(PointSet& ps, PointAttribute<bool>& visible, PointAttribute<float>& value, PointAttribute<int>& primitive_id);
	void render();
};

struct SegmentRenderer: public Renderer{
	GLfloat width_range[2];
	struct Vertex {
		std::array<float, 3> p0;
		float v0;
		std::array<float, 3> p1;
		float v1;
		int visible;
		int primitive_id;
	};

	int line_width=10;
	float origin_scale = 1.;

	void generate_gui(std::string name);
	void init();
	void update(PolyLine& pl, EdgeAttribute<bool>& visible, PointAttribute<float>& value, EdgeAttribute<int>& primitive_id);
	void render();
};


struct TriangleRenderer: public Renderer{
	int cull_mode = 0;
	int edge_width = 1;
	bool show_edge = true;
	float edge_color[3] = { 0,0,1 };

	struct Vertex {
		std::array<float, 3> pos;
		std::array<float, 3> n; // normal
		float v;                // value
		std::array<float, 3> b; // bary
		int visible;
		int primitive_id;
	};

	void generate_gui(std::string name);

	void init();

	void update(Triangles& tri, FacetAttribute<bool>& visible, CornerAttribute<float>& value, FacetAttribute<int>& primitive_id);

	void render();
};

