#include "colormap.h"

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





struct SimplexRenderer{
	GLuint vao,vbo;
	int npts;
	GLuint colormap;
	int texture_repeat=4;
	int texture_id=0;
	float color[3] = {.5,.8,.5};
	float color_map_prop=1;
	float ambient_prop=.5;
	GLuint shaderProgram;
	float light_direction[3] = { 1,1,1 };
	int layer_id = -1;

	SimplexRenderer(int layer_id) : layer_id(layer_id) {}

	void generate_gui(std::string name){
		ImGui::PushItemWidth(120);
		ImGui::SliderFloat(("ambient_M##slider"+name).c_str(),&ambient_prop,0.0f,1.0f,"%.3f",0);
		ImGui::SliderFloat(("color/texture##slider"+name).c_str(),&color_map_prop,0.0f,1.0f,"%.3f",0);

		// need the constant color
		if(color_map_prop<1)
			ImGui::ColorEdit3(("ConstColor##"+name).c_str(),(float*)&color,ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		// need a colormap
		if(color_map_prop>0){
			int prev = texture_id;
			ColormapCombo("Colormap", texture_id);
			if (texture_id!=prev) {
				glDeleteTextures(1, &colormap);
				load_colormap(texture_id, colormap);
			}
			ImGui::InputInt("#texture_repeat",&texture_repeat);
			texture_repeat = std::clamp(texture_repeat,1,1000000);
		}

		ImGui::PopItemWidth();
	}

	void shared_setup_before_rendering(){
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glUseProgram(shaderProgram);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D,colormap);
		glBindVertexArray(vao);
		glUniform1f(glGetUniformLocation(shaderProgram,"texture_repeat"),texture_repeat);
		glUniform1f(glGetUniformLocation(shaderProgram,"color_map_prop"),color_map_prop);
		glUniform1f(glGetUniformLocation(shaderProgram,"ambient_prop"),ambient_prop);
		glUniform1i(glGetUniformLocation(shaderProgram,"colormap"),0);
		glUniform3fv(glGetUniformLocation(shaderProgram,"color"),1,color);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_TRUE, God::camera.view());
		auto [w, h] = God::context.screen_size();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_TRUE, God::camera.projection());
		glUniform3fv(glGetUniformLocation(shaderProgram, "light_direction"), 1, light_direction);
		glUniform1i(glGetUniformLocation(shaderProgram,"layer_id"),layer_id);
	}

	// declare uniforms for raytraced primitives (sphere and cylinder)
	void declare_inv_projection_matrix() {
		auto [w, h] = God::context.screen_size();
		mat4x4 inv_proj = God::camera.impl->projection_matrix().invert();
		static float inv_proj_float[16]; FOR(i, 16) inv_proj_float[i] = inv_proj[i / 4][i % 4];
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "inv_projection"), 1, GL_TRUE, inv_proj_float);
	}
	void declare_viewport() {
		auto [w, h] = God::context.screen_size();
		glUniform2f(glGetUniformLocation(shaderProgram, "viewport"), float(w), float(h));
	}
};







struct PointRenderer: public SimplexRenderer{
	int radius_in_pixel=2;

	using SimplexRenderer::SimplexRenderer;

	void generate_gui(std::string name){
		ImGui::PushItemWidth(80);
		ImGui::InputInt("point size",&radius_in_pixel);
		ImGui::PopItemWidth();
		SimplexRenderer::generate_gui(name);
	}

	void init_from_mesh(PointSet &ps,PointAttribute<float>& value){
		std::vector<float> vertices(4*ps.size(),0);
		FOR(v,ps.size()){
			FOR(d,3) vertices[4*v+d] = ps[v][d];
			vertices[4*v+3] = value[v];
		}
		init(vertices.data(),ps.size());
	}

	void init(float* pts,int pts_size){
		if (!God::shaders.contains("point_as_sphere"))
			God::shaders.add(std::string(SHADERS_DIR),"point_as_sphere");
		shaderProgram=God::shaders["point_as_sphere"];
		load_colormap(texture_id, colormap);
		npts = pts_size;
		glGenVertexArrays(1,&vao);
		glGenBuffers(1,&vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER,4*pts_size * sizeof(float),pts,GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(3 * sizeof(float)));
		glBindVertexArray(0);
		um_assert(no_gl_error());
	}


	void render(){
		um_assert(no_gl_error());
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_PROGRAM_POINT_SIZE);
		shared_setup_before_rendering();
		declare_inv_projection_matrix();
		declare_viewport();

		auto [w,h] = God::context.screen_size();
		float pointRadius = 2.*double(radius_in_pixel)/(God::camera.impl->projection_matrix()[1][1]*double(h));

		glUniform1f(glGetUniformLocation(shaderProgram,"R"),pointRadius);
		glUniform3fv(glGetUniformLocation(shaderProgram,"color"),1,color);

		glBindVertexArray(vao);

		glDrawArrays(GL_POINTS,0,GLsizei(npts));
		glBindVertexArray(0);

		um_assert(no_gl_error());
	}

};
struct SegmentRenderer: public SimplexRenderer{
	GLfloat range[2];

	using SimplexRenderer::SimplexRenderer;

	int line_width=10;
	float origin_scale = 1.;
	void generate_gui(std::string name){
		ImGui::PushItemWidth(80);
		ImGui::InputInt("line width",&line_width);
		line_width = std::clamp(line_width,int(range[0]),int(range[1]));
		ImGui::SliderFloat(("origin scale##slider"+name).c_str(),&origin_scale,0.4f,1.0f,"%.3f",0);
		ImGui::PopItemWidth();
		SimplexRenderer::generate_gui(name);
	}


	void init_from_mesh(PolyLine& pl,PointAttribute<float>& value){
		std::vector<float> vertices(16*pl.nedges(),0);
		for(auto e:pl.iter_edges()){
			FOR(d,3) vertices[16*e+d] = e.from().pos()[d];
			vertices[16*e+3] = value[e.from()];
			FOR(d,3) vertices[16*e+4+d] = e.to().pos()[d];
			vertices[16*e+7] = value[e.to()];
			FOR(i,8) vertices[16*e+8+i]=vertices[16*e+i];
		}
		init(vertices.data(),pl.nedges());
	}

	void init(float* pts,int nedges){
		glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE,range);
		if(!God::shaders.contains("segment_as_tube"))
			God::shaders.add(std::string(SHADERS_DIR),"segment_as_tube");
		shaderProgram=God::shaders["segment_as_tube"];

		load_colormap(texture_id, colormap);
		um_assert(no_gl_error());
		npts = 2*nedges;
		glGenVertexArrays(1,&vao);
		glGenBuffers(1,&vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);

		glBufferData(GL_ARRAY_BUFFER,8*npts* sizeof(float),pts,GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(4 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(7 * sizeof(float)));
		glBindVertexArray(0);
	}




	void render(){
		um_assert(no_gl_error());
		glEnable(GL_DEPTH_TEST);
		glUseProgram(shaderProgram);

		auto [w,h] = God::context.screen_size();
		line_width = std::min(line_width,int(range[1]));
		double radius = float(line_width) /(2.0f * God::camera.impl->projection_matrix()[1][1]* float(h));
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
};


struct TriangleRenderer: public SimplexRenderer{

	using SimplexRenderer::SimplexRenderer;

	void init_from_mesh(Triangles& tri,CornerAttribute<float>& value){
		std::vector<float> data(7*tri.ncorners());
		for(auto h:tri.iter_halfedges())  {
			int h_id = h;
			int lh = h_id%3;
			vec3 n = Triangle3(h.facet()).normal();
			FOR(d,3) data[7*h_id + d] = h.from().pos()[d];
			FOR(d,3) data[7*h_id +3+ d] = n[d];
			data[7*h_id +6] = value[h];
		}
		
		init(data.data(),tri.nfacets());
	}

	void init(float* pts,int ntriangles){
		color_map_prop=0;
		if(!God::shaders.contains("triangle"))
			God::shaders.add(std::string(SHADERS_DIR),"triangle");
		shaderProgram=God::shaders["triangle"];
		load_colormap(0,colormap);
		npts = ntriangles*3;

		glGenVertexArrays(1,&vao);
		glGenBuffers(1,&vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo);
		glBufferData(GL_ARRAY_BUFFER,7*npts* sizeof(float),pts,GL_STATIC_DRAW);

		// position
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,7 * sizeof(float),(void*)0);
		// normale
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,7 * sizeof(float),(void*)(3 * sizeof(float)));
		// value
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,7 * sizeof(float),(void*)(6 * sizeof(float)));

		glBindVertexArray(0);
		um_assert(no_gl_error());
	}

	void render(){
		um_assert(no_gl_error());
		shared_setup_before_rendering();
		glDrawArrays(GL_TRIANGLES,0,GLsizei(npts));
		glBindVertexArray(0);
		um_assert(no_gl_error());
	}
};

