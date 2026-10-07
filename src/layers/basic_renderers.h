#pragma once
#include "colormap.h"
#include <array>
#include <string>
#include <glad/gl.h>
#include <ultimaille/all.h>
using namespace UM;

bool no_gl_error();


struct SimplexRenderer{
	GLuint vao,vbo;
	// GLuint visible_buf, visible_tex;
	int npts;
	GLuint shaderProgram;
	float light_direction[3] = { 1,1,1 };
	int layer_id = -1;

	GLuint colormap;
	int texture_repeat=1;
	int texture_id=0;
	float data_autorange[2] = {0,0};
	float data_range[2] = {0,0};
	float color[3] = {.8,.8,.8};
	float color_map_prop=0;
	float ambient_prop=.5;

	struct Clipping {
		int mode = 1; // {0 = cell, 1 = std, 2 = slice}
		float normal[3] = {0,1,0};
		float pos[3] = {0.5,0.5,0.5};
		bool invert{false};
		bool enabled{false};
	} clipping;

	SimplexRenderer(int layer_id);

	void generate_gui(std::string name);

	void shared_setup_before_rendering();

	// declare uniforms for raytraced primitives (sphere and cylinder)
	void declare_inv_projection_matrix();
	void declare_viewport();

	virtual void destroy();

	protected:
	// TODO probably move this elsewhere
		std::array<float, 2> range(std::vector<float>& data);

		void compute_range(std::vector<float>& data);

		constexpr std::array<float, 3> to_float3(const vec3& v);
};







struct PointRenderer: public SimplexRenderer{
	int radius_in_pixel=2;

	using SimplexRenderer::SimplexRenderer;

	struct Vertex {
		std::array<float, 3> pos;
		float v;
		bool visible;
	};

	void generate_gui(std::string name);

	void init();

	void update(PointSet& ps, PointAttribute<bool>& visible, PointAttribute<float>& value);


	void render();

};
struct SegmentRenderer: public SimplexRenderer{
	GLfloat range[2];

	using SimplexRenderer::SimplexRenderer;

	struct Vertex {
		std::array<float, 3> p0;
		float v0;
		std::array<float, 3> p1;
		float v1;
		bool visible;
	};

	int line_width=10;
	float origin_scale = 1.;
	void generate_gui(std::string name);

	void init();

	void push(PolyLine& pl, EdgeAttribute<bool>& visible, PointAttribute<float>& value);



	void render();
};


struct TriangleRenderer: public SimplexRenderer{

	using SimplexRenderer::SimplexRenderer;
	int cull_mode = 0;
	int edge_width = 1;
	bool show_edge = true;
	float edge_color[3] = { 0,0,1 };




	struct Vertex {
		std::array<float, 3> pos;
		std::array<float, 3> n; // normal
		float v; // value
		std::array<float, 3> b; // bary
		bool visible;
		// int primitive_id;
	};

	void generate_gui(std::string name);

	void init();

	void update(Triangles& tri, FacetAttribute<bool>& visible, CornerAttribute<float>& value);

	void update(Quads& quads, FacetAttribute<bool>& visible, CornerAttribute<float>& value);

	void update(Tetrahedra& tet, CellAttribute<bool>& visible, CellCornerAttribute<float>& value);

	void update(Hexahedra& hex, CellAttribute<bool>& visible, CellCornerAttribute<float>& value);

	void render();
};

