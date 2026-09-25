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

struct Renderer{
	GLuint shaderProgram;
	float light_direction[3] = {1,1,1};


	void declare_projection_matrix(){
		auto [w,h] = God::context.screen_size();
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"),1,GL_TRUE,God::camera.projection());
	}
	void declare_inv_projection_matrix(){
		auto [w,h] = God::context.screen_size();
		mat4x4 inv_proj = God::camera.impl->projection_matrix().invert();
		static float inv_proj_float[16]; FOR(i,16) inv_proj_float[i] = inv_proj[i/4][i%4];
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"inv_projection"),1,GL_TRUE,inv_proj_float);
	}
	void declare_view_matrix(){
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_TRUE,God::camera.view());
	}

	void declare_light_direction(){
		glUniform3fv(glGetUniformLocation(shaderProgram,"light_direction"),1,light_direction);
	}
	void declare_viewport(){
		auto [w,h] = God::context.screen_size();
		glUniform2f(glGetUniformLocation(shaderProgram,"viewport"),float(w),float(h));
	}
};




struct SimplexRenderer: public Renderer{
	GLuint vao,vbo;
	int npts;
	GLuint colormap;
	int texture_repeat=4;
	int texture_id=0;
	float color[3] = {.5,.8,.5};
	float color_map_prop=1;
	float ambient_prop=.5;

	void generate_gui(std::string name){

		ImGui::PushItemWidth(80);
		ImGui::SliderFloat(("ambient_M##slider"+name).c_str(),&ambient_prop,0.0f,1.0f,"%.3f",0);
		ImGui::SliderFloat(("color/texture##slider"+name).c_str(),&color_map_prop,0.0f,1.0f,"%.3f",0);

		if(color_map_prop<1)
			ImGui::ColorEdit3(("ConstColor##"+name).c_str(),(float*)&color,ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);

		if(color_map_prop>0){
			int prev_color_map=texture_id;
			ImGui::InputInt("#textureid##texture_id",&texture_id);
			texture_id = std::clamp(texture_id,0,2);
			if(texture_id!=prev_color_map){
				glDeleteTextures(1,&colormap);
				init_colormap(texture_id);
			}
			ImGui::InputInt("#texture_repeat",&texture_repeat);
			texture_repeat = std::clamp(texture_repeat,1,1000000);
		}

		ImGui::PopItemWidth();

	}

	void init_colormap(int id){
		glGenTextures(1,&colormap);
		glBindTexture(GL_TEXTURE_1D,colormap);
		glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_WRAP_S,GL_REPEAT);

		std::vector<unsigned char> colors_viridis={
			68,1,84,70,7,90,71,13,96,71,19,101,72,24,106,72,29,111,72,35,116,72,40,120,71,45,123,70,50,126,69,55,129,68,59,132,66,64,134,64,69,136,62,73,137,
			61,78,138,58,83,139,56,88,140,54,92,141,52,96,141,50,100,142,49,104,142,47,108,142,45,112,142,44,115,142,42,119,142,41,123,142,39,127,142,38,130,142,36,134,142,35,138,141,
			33,142,141,32,146,140,31,150,139,31,154,138,31,158,137,31,161,135,33,165,133,35,169,131,38,173,129,42,176,127,47,180,124,53,183,121,59,187,117,66,190,113,74,193,109,82,197,105,
			90,200,100,101,203,94,110,206,88,119,209,83,129,211,77,139,214,70,149,216,64,160,218,57,170,220,50,181,222,43,192,223,37,202,225,31,213,226,26,223,227,24,234,229,26,244,230,30,
			253,231,37
		};

		std::vector<unsigned char> colors_cividis={
			0,34,78,0,37,84,0,40,91,0,43,98,0,46,106,0,48,112,5,51,113,18,53,112,26,56,111,33,59,110,39,62,110,45,65,109,50,67,109,54,70,108,59,73,108,
			63,76,108,68,79,108,72,82,108,76,85,108,80,87,108,84,90,109,87,93,109,91,96,110,94,99,111,98,101,111,101,104,112,105,107,113,108,110,114,112,113,115,115,116,117,119,119,118,
			122,122,120,126,125,120,130,128,121,134,131,121,138,134,120,142,137,120,146,140,120,149,143,119,153,146,119,157,149,118,161,153,117,165,156,116,169,159,115,173,162,114,177,165,112,182,169,111,
			186,172,109,191,176,107,195,179,105,199,183,103,204,186,100,208,190,98,212,193,95,217,197,92,221,200,88,225,204,85,230,208,81,234,211,76,239,215,72,243,219,66,248,223,60,253,227,52,
			254,232,56
		};

		std::vector<unsigned char> colors_blues={
			247,251,255,244,249,254,241,247,253,237,245,252,234,243,251,231,241,250,228,239,249,225,237,248,222,235,247,219,233,246,216,231,245,212,229,244,209,227,243,206,225,242,203,223,241,
			200,221,240,197,218,239,192,216,237,187,214,235,181,212,233,176,210,231,171,208,230,166,206,228,161,203,226,156,201,224,149,197,223,143,194,222,136,190,220,130,186,219,123,183,217,117,179,216,
			110,176,215,104,172,213,99,169,211,94,165,209,89,162,207,84,158,205,78,154,203,73,151,201,68,147,199,63,143,197,59,139,194,55,135,192,51,131,190,47,127,188,42,122,186,38,118,184,
			34,114,182,31,110,179,27,106,175,24,102,172,21,98,169,18,94,166,15,90,163,12,86,160,8,82,156,8,77,151,8,73,144,8,69,138,8,65,132,8,61,126,8,56,119,8,52,113,
			8,48,107
		};
		switch(id){
		case 0: glTexImage1D(GL_TEXTURE_1D,0,GL_RGB8,64,0,GL_RGB,GL_UNSIGNED_BYTE,colors_viridis.data()); return;
		case 1: glTexImage1D(GL_TEXTURE_1D,0,GL_RGB8,64,0,GL_RGB,GL_UNSIGNED_BYTE,colors_cividis.data()); return;
		case 2: glTexImage1D(GL_TEXTURE_1D,0,GL_RGB8,64,0,GL_RGB,GL_UNSIGNED_BYTE,colors_blues.data()); return;
		}
	}
	void shared_setup_before_rendering(){
		glEnable(GL_DEPTH_TEST);
		glUseProgram(shaderProgram);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D,colormap);
		glBindVertexArray(vao);
		glUniform1f(glGetUniformLocation(shaderProgram,"texture_repeat"),texture_repeat);
		glUniform1f(glGetUniformLocation(shaderProgram,"color_map_prop"),color_map_prop);
		glUniform1f(glGetUniformLocation(shaderProgram,"ambient_prop"),ambient_prop);
		glUniform1i(glGetUniformLocation(shaderProgram,"colormap"),0);
		glUniform3fv(glGetUniformLocation(shaderProgram,"color"),1,color);
		declare_projection_matrix();
		declare_view_matrix();
		declare_light_direction();

	}
};







struct PointRenderer: public SimplexRenderer{
	int radius_in_pixel=2;


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
		God::shaders.add(std::string(SHADERS_DIR),"point_as_sphere");
		shaderProgram=God::shaders["point_as_sphere"];
		init_colormap(0);
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

	int line_width=10;
	float origin_scale = 1.;
	void generate_gui(std::string name){
		ImGui::PushItemWidth(80);
		ImGui::InputInt("line width",&line_width);
		line_width = std::clamp(line_width,int(range[0]),int(range[1]));
		ImGui::SliderFloat(("origin scale##slider"+name).c_str(),&origin_scale,0.5f,1.0f,"%.3f",0);
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
		God::shaders.add(std::string(SHADERS_DIR),"segment_as_tube");
		shaderProgram=God::shaders["segment_as_tube"];

		init_colormap(0);
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
		shaderProgram=God::shaders["triangle"];
		init_colormap(0);
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

