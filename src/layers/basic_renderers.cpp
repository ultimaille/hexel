#include "basic_renderers.h"
#include <core/basic.h>
#include <array>
#include <glad/gl.h>
#include "basic_renderers.h"
#include "core/layers.h"
#include "core/camera.h"
#include "core/picker.h"
#include "core/xcf.h"
#include "core/render_target.h"
#include "core/window.h"
#include "core/panels.h"
#include "core/layers.h"
#include "core/shaders.h"
#include "core/mode.h"

bool no_gl_error() {
	switch(glGetError()){
	case GL_INVALID_ENUM:					Log::error("GL_INVALID_ENUM"); return false;
	case GL_INVALID_VALUE:					Log::error("GL_INVALID_VALUE"); return false;
	case GL_INVALID_OPERATION:				Log::error("GL_INVALID_OPERATION"); return false;
	case GL_INVALID_FRAMEBUFFER_OPERATION:	Log::error("GL_INVALID_FRAMEBUFFER_OPERATION"); return false;
	case GL_OUT_OF_MEMORY:					Log::error("GL_OUT_OF_MEMORY"); return false;
	}
	return true;
}







SimplexRenderer::SimplexRenderer(int layer_id) : layer_id(layer_id) {}

	void SimplexRenderer::generate_gui(std::string name){
		ImGui::PushItemWidth(120);
		ImGui::SliderFloat(label("ambient_M##slider",name),&ambient_prop,0.0f,1.0f,"%.3f",0);
		ImGui::SliderFloat(label("color/texture##slider",name), &color_map_prop, 0.0f, 1.0f, "%.3f", 0);

		// need the constant color
		if(color_map_prop<1)
			ImGui::ColorEdit3(label("ConstColor##",name),(float*)&color,ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		// need a colormap
		if(color_map_prop>0){
			int prev = texture_id;
			ColormapCombo("Colormap", texture_id);
			if (texture_id!=prev) {
				glDeleteTextures(1, &colormap);
				load_colormap(texture_id, colormap);
			}
			ImGui::InputFloat2(label("range##range",name), data_range);
			if (ImGui::InputInt(label("texture repeat##texture_repeat",name),&texture_repeat)) {
				if (texture_repeat > 1) {
					glBindTexture(GL_TEXTURE_1D, colormap);
					glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				} else {
					glBindTexture(GL_TEXTURE_1D, colormap);
					glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				}
					texture_repeat = std::clamp(texture_repeat,1,1000000);
			}
			if (ImGui::SmallButton(label("autorange##autorange",name))) {
				std::copy(data_autorange, data_autorange + 2, data_range);
			}

		}
		ImGui::Checkbox(label("clip",name), &clipping.enabled);

		ImGui::PopItemWidth();
	}

	void SimplexRenderer::shared_setup_before_rendering(){
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glUseProgram(shaderProgram);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D,colormap);
		glBindVertexArray(vao);
		glUniform1f(glGetUniformLocation(shaderProgram,"texture_repeat"),texture_repeat);
		glUniform2fv(glGetUniformLocation(shaderProgram,"data_range"),1, data_range);
		glUniform1f(glGetUniformLocation(shaderProgram,"color_map_prop"),color_map_prop);
		glUniform1f(glGetUniformLocation(shaderProgram,"ambient_prop"),ambient_prop);
		glUniform1i(glGetUniformLocation(shaderProgram,"colormap"),0);
		glUniform3fv(glGetUniformLocation(shaderProgram,"color"),1,color);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_TRUE, God::camera.view_ptr());
		auto [w, h] = God::context.screen_size();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_TRUE, God::camera.projection_ptr());
		glUniform3fv(glGetUniformLocation(shaderProgram, "light_direction"), 1, light_direction);

		glUniform1i(glGetUniformLocation(shaderProgram,"clipping.mode"), clipping.mode);
		glUniform3fv(glGetUniformLocation(shaderProgram,"clipping.normal"), 1, clipping.normal);
		glUniform3fv(glGetUniformLocation(shaderProgram,"clipping.pos"), 1, clipping.pos);
		glUniform1i(glGetUniformLocation(shaderProgram,"clipping.invert"), clipping.invert);
		glUniform1i(glGetUniformLocation(shaderProgram,"clipping.enabled"), clipping.enabled);

		glUniform1i(glGetUniformLocation(shaderProgram,"layer_id"),layer_id);
	}

	// declare uniforms for raytraced primitives (sphere and cylinder)
	void SimplexRenderer::declare_inv_projection_matrix() {
		auto [w, h] = God::context.screen_size();
		mat4x4 inv_proj = God::camera.projection_matrix().invert();
		static float inv_proj_float[16]; FOR(i, 16) inv_proj_float[i] = inv_proj[i / 4][i % 4];
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "inv_projection"), 1, GL_TRUE, inv_proj_float);
	}
	void SimplexRenderer::declare_viewport() {
		auto [w, h] = God::context.screen_size();
		glUniform2f(glGetUniformLocation(shaderProgram, "viewport"), float(w), float(h));
	}

	 void SimplexRenderer::destroy() {
		um_assert(no_gl_error());
		if (vao != 0) {
			glDeleteVertexArrays(1, &vao);
		}
		if (vbo != 0) {
			glDeleteBuffers(1, &vbo);
		}

		um_assert(no_gl_error());
	}


	// TODO probably move this elsewhere
	std::array<float, 2> SimplexRenderer::range(std::vector<float>& data) {
		float min = std::numeric_limits<float>::max(); 
		float max = std::numeric_limits<float>::min();
		for (int i = 0; i < data.size(); ++i) {
			auto x = data[i];
			min = std::min(min, x);
			max = std::max(max, x);
		}

		return {min, max};
	}

	void SimplexRenderer::compute_range(std::vector<float>& data) {
		auto r = range(data);
		std::copy(r.begin(), r.end(), data_autorange);
		std::copy(data_autorange, data_autorange + 2, data_range);
	}

	constexpr std::array<float, 3> SimplexRenderer::to_float3(const vec3 &v) {
		return {
			static_cast<float>(v.x),
			static_cast<float>(v.y),
			static_cast<float>(v.z)
		};
	}









	//using SimplexRenderer::SimplexRenderer;


	void PointRenderer::generate_gui(std::string name){
		ImGui::PushItemWidth(80);
		ImGui::InputInt(("point size##point_size"+name).c_str(),&radius_in_pixel);
		ImGui::PopItemWidth();
		SimplexRenderer::generate_gui(name);
	}

	void PointRenderer::init(){
		if (!God::shaders.contains("point_as_sphere"))
			God::shaders.add(std::string(SHADERS_DIR),"point_as_sphere");
		shaderProgram=God::shaders["point_as_sphere"];
		load_colormap(texture_id, colormap);
		glGenVertexArrays(1,&vao);
		glGenBuffers(1,&vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, v));
		glEnableVertexAttribArray(2);
		glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, visible));

		glBindVertexArray(0);
		um_assert(no_gl_error());
	}

	void PointRenderer::update(PointSet& ps, PointAttribute<bool> &visible, PointAttribute<float>& value) {
		compute_range(value.ptr->data);

		npts = ps.size();
		std::vector<Vertex> vertices(ps.size());
		FOR(v,ps.size()){
			vertices[v] = {
				.pos = to_float3(ps[v]),
				.v = value[v],
				.visible = visible[v]
			};
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	}


	void PointRenderer::render(){
		um_assert(no_gl_error());
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_PROGRAM_POINT_SIZE);
		shared_setup_before_rendering();
		declare_inv_projection_matrix();
		declare_viewport();

		auto [w,h] = God::context.screen_size();
		float pointRadius = 2.*double(radius_in_pixel)/(God::camera.projection_matrix()[1][1]*double(h));

		glUniform1f(glGetUniformLocation(shaderProgram,"R"),pointRadius);
		glUniform3fv(glGetUniformLocation(shaderProgram,"color"),1,color);

		glBindVertexArray(vao);

		glDrawArrays(GL_POINTS,0,GLsizei(npts));
		glBindVertexArray(0);

		um_assert(no_gl_error());
	}



	void SegmentRenderer::generate_gui(std::string name){
		ImGui::PushItemWidth(80);
		ImGui::InputInt("line width",&line_width);
		line_width = std::clamp(line_width,int(range[0]),int(range[1]));
		ImGui::SliderFloat(("origin scale##slider"+name).c_str(),&origin_scale,0.4f,1.0f,"%.3f",0);
		ImGui::PopItemWidth();
		SimplexRenderer::generate_gui(name);
	}

	void SegmentRenderer::init(){
		glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE,range);
		if(!God::shaders.contains("segment_as_tube"))
			God::shaders.add(std::string(SHADERS_DIR),"segment_as_tube");
		shaderProgram=God::shaders["segment_as_tube"];

		load_colormap(texture_id, colormap);
		um_assert(no_gl_error());
		glGenVertexArrays(1,&vao);
		glGenBuffers(1,&vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, p0));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, v0));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, p1));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, v1));
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, visible));

		glBindVertexArray(0);
	}

	void SegmentRenderer::push(PolyLine& pl, EdgeAttribute<bool> &visible, PointAttribute<float>& value) {
		compute_range(value.ptr->data);

		npts = 2*pl.nedges();
		std::vector<Vertex> vertices(npts);
		for(auto e : pl.iter_edges()){
			auto p0 = e.from().pos();
			auto p1 = e.to().pos();

			Vertex v{
				.p0 = to_float3(p0),
				.v0 = value[e.from()],
				.p1 = to_float3(p1),
				.v1 = value[e.to()],
				.visible = visible[e]
			};
			vertices[e * 2] = v;
			vertices[e * 2 + 1] = v;
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(),GL_STATIC_DRAW);
	}



	void SegmentRenderer::render(){
		um_assert(no_gl_error());
		glEnable(GL_DEPTH_TEST);
		glUseProgram(shaderProgram);

		auto [w,h] = God::context.screen_size();
		line_width = std::min(line_width,int(range[1]));
		double radius = float(line_width) /(2.0f * God::camera.projection_matrix()[1][1]* float(h));
		glLineWidth(line_width);

		shared_setup_before_rendering();
		declare_inv_projection_matrix();
		declare_viewport();

		glUniform1f(glGetUniformLocation(shaderProgram,"R_dest"),radius);
		glUniform1f(glGetUniformLocation(shaderProgram,"R_org"),origin_scale * radius);
		glUniform3fv(glGetUniformLocation(shaderProgram,"color"),1,color);
		glBindVertexArray(vao);
		glDrawArrays(GL_LINES,0,GLsizei(npts));
		glBindVertexArray(0);
		um_assert(no_gl_error());
	}








	void TriangleRenderer::generate_gui(std::string name){
		SimplexRenderer::generate_gui(name);
		static const char* cull_modes[] = { "NONE","BACK","FRONT" };
		const char* combo_preview_value = cull_modes[cull_mode];
		if (ImGui::BeginCombo(label("CULL " + std::string(cull_modes[cull_mode])), combo_preview_value, ImGuiComboFlags_NoPreview)){
			for (int n = 0; n < 3; n++){
				const bool is_selected = (cull_mode == n);
				if (ImGui::Selectable(cull_modes[n], is_selected)) cull_mode = n;
				if (is_selected)ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}																		   

	void TriangleRenderer::init(){
		color_map_prop=0;
		if(!God::shaders.contains("triangle"))
			God::shaders.add(std::string(SHADERS_DIR),"triangle");
		shaderProgram=God::shaders["triangle"];
		load_colormap(0,colormap);

		glGenVertexArrays(1,&vao);
		glGenBuffers(1,&vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);

		// position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
		// normal
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT,GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, n));
		// value
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 1, GL_FLOAT,GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, v));
		// bary
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT,GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, b));
		// visible
		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, visible));
		// // primitive id
		// glEnableVertexAttribArray(5);
		// glVertexAttribIPointer(5, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, primitive_id));

		glBindVertexArray(0);

		// visible
		// glGenBuffers(1, &visible_buf);
		// glGenTextures(1, &visible_tex);
		// glBindBuffer(GL_TEXTURE_BUFFER, visible_buf);
		// glBindTexture(GL_TEXTURE_BUFFER, visible_tex);
		// glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, visible_buf);

		um_assert(no_gl_error());
	}

	void TriangleRenderer::update(Triangles& tri, FacetAttribute<bool>& visible, CornerAttribute<float>& value) {
		compute_range(value.ptr->data);

		std::vector<Vertex> vertices(tri.ncorners());
		npts = vertices.size();

		for(auto f : tri.iter_facets())  {
			auto t = Triangle3(f);
			auto n = t.normal();
			auto b = t.bary_verts();

			for (int lv = 0; lv < 3; ++lv) {
				auto h = f * 3 + lv;
				vec3 p = f.vertex(lv);
				vertices[h] = {
					.pos = to_float3(p),
					.n = to_float3(n),
					.v = value[h],
					.b = to_float3(b),
					.visible = visible[f],
					// .primitive_id = f
				};
			}
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		// std::vector<int> ivisible(visible.ptr->data.begin(), visible.ptr->data.end());
		// glBindBuffer(GL_ARRAY_BUFFER, visible_buf);
		// glBufferData(GL_ARRAY_BUFFER, ivisible.size() * sizeof(int), ivisible.data(), GL_STATIC_DRAW);
	}

	void TriangleRenderer::update(Quads& quads, FacetAttribute<bool> &visible, CornerAttribute<float>& value) {
		compute_range(value.ptr->data);

		std::vector<Vertex> vertices(quads.ncorners() * 3);
		npts = vertices.size();

		for(auto f : quads.iter_facets())  {
			auto t = Quad3(f);
			auto n = t.normal();
			auto b = t.bary_verts();

			float bary_val = 0;
			for (int lv = 0; lv < 4; ++lv) bary_val += value[f * 4 + lv];
			bary_val /= 4;

			for (int lv = 0; lv < 4; ++lv) {
				auto h = f * 4 + lv;
				float v[3] = {bary_val, value[f * 4 + lv], value[f * 4 + (lv + 1) % 4]};
				vec3 points[3] = {b, f.vertex(lv), f.vertex((lv + 1) % 4)};

				for (int i = 0; i < 3; ++i) {
					vertices[f * 12 + (lv * 3 + i)] = {
						.pos = to_float3(points[i]),
						.n = to_float3(n),
						.v = v[i],
						.b = to_float3(b),
						.visible = visible[f]
					};
				}
			}
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	}

	void TriangleRenderer::update(Tetrahedra& tet, CellAttribute<bool> &visible, CellCornerAttribute<float>& value) {
		compute_range(value.ptr->data);

		std::vector<Vertex> vertices(tet.nfacets() * 3);
		npts = vertices.size();

		for (auto c : tet.iter_cells()) {
			auto t = Tetrahedron(c);
			auto b = t.bary_verts();

			for(auto f : c.iter_facets())  {
				auto tri = Triangle3(f);
				auto n = tri.normal();

				for (int lv = 0; lv < 3; ++lv) {
					auto h = f * 3 + lv;
					vec3 p = f.vertex(lv);
					vertices[h] = {
						.pos = to_float3(p),
						.n = to_float3(n),
						.v = value[f.corner(lv)],
						.b = to_float3(b),
						.visible = visible[c]
					};
				}
			}
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),vertices.data(),GL_STATIC_DRAW);
	}

	void TriangleRenderer::update(Hexahedra& hex, CellAttribute<bool> &visible, CellCornerAttribute<float>& value) {
		compute_range(value.ptr->data);

		std::vector<Vertex> vertices(hex.nfacets() * 12 /* 4 tri per quad, 3 verts per tri */);
		npts = vertices.size();

		for (auto c : hex.iter_cells()) {
			auto t = Hexahedron(c);
			auto b = t.bary_verts();

			for(auto f : c.iter_facets())  {
				auto tri = Quad3(f);
				auto n = tri.normal();

				float bary_val = 0;
				for (int lv = 0; lv < 4; ++lv) bary_val += value[f.corner(lv)];
				bary_val /= 4;

				for (int lv = 0; lv < 4; ++lv) {
					
					auto c0 = f.corner(lv);
					auto c1 = f.corner((lv + 1) % 4);
					float v[3] = {bary_val, value[c0], value[c1]};
					// float v[3] = {0,0,0};
					vec3 points[3] = {b, f.vertex(lv), f.vertex((lv + 1) % 4)};

					for (int i = 0; i < 3; ++i) {
						vertices[f * 12 + (lv * 3 + i)] = {
							.pos = to_float3(points[i]),
							.n = to_float3(n),
							.v = v[i],
							.b = to_float3(b),
							.visible = visible[c]
						};
					}
				}


			}
		}

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),vertices.data(),GL_STATIC_DRAW);
	}

	void TriangleRenderer::render(){
		um_assert(no_gl_error());
		if (cull_mode>0){
			glEnable(GL_CULL_FACE);
			if (cull_mode == 1) 
				glCullFace(GL_BACK);
			else  
				glCullFace(GL_FRONT);
		}

		shared_setup_before_rendering();
		
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawArrays(GL_TRIANGLES,0,GLsizei(npts));


		glBindVertexArray(0);




		glDisable(GL_CULL_FACE);
		um_assert(no_gl_error());
	}


