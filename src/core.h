#include <set>
using namespace UM;


struct Picker{
	float* data; int w;int h; // any image structure would work
	//Picker(){God::layers.produce_picking_image(data);}
	//std::tuple<int,int> at(vec2 uv){
		//...
		//return {layerid,primitiveid};
	//}
	
};

// -------------------------------------------------------------------------------
//                                    CAMERA
// -------------------------------------------------------------------------------
// => Access direct dans God::camera
// => possibilité de changer via l'implementation

struct CameraInterface{
	virtual mat4x4  projection_matrix(float width,float height)=0;
	virtual mat4x4  view_matrix()=0;
	virtual void update()=0;
};

struct OrthographicCamera: public CameraInterface{
	double zoom = 1.;
	double rotX = 0.;
	double rotY = 0.;

	mat4x4 ortho(double  left,double right,double bottom,double top,double zNear,double zFar){
		mat4x4 m;
		m[0][0] = 2. / (right - left);
		m[1][1] = 2. / (top - bottom);
		m[2][2] = - 2. / (zFar - zNear);
		m[3][0] = - (right + left) / (right - left);
		m[3][1] = - (top + bottom) / (top - bottom);
		m[3][2] = - (zFar + zNear) / (zFar - zNear);
		return m;
	}

	virtual mat4x4 projection_matrix(float width,float height) {
		mat4x4 m = mat4x4::identity();
		m[0][0] = height/width;
		m[2][2] = .5;
		FOR(d,3)m[d][d]*=zoom;
		return m;
	}

	virtual mat4x4 view_matrix(){
		mat4x4 rx= mat4x4::identity();
		mat4x4 ry= mat4x4::identity();
		{
			double s = std::sin(rotX);
			double c = std::cos(rotX);
			rx[1][1] = c;  rx[1][2] = s;
			rx[2][1] = -s; rx[2][2] = c;
		}
		{
			double s = std::sin(rotY);
			double c = std::cos(rotY);
			ry[0][0] = c;  ry[0][2] = s;
			ry[2][0] = -s; ry[2][2] = c;
		}
		return rx*ry;
	}
	virtual void update();

};





struct Camera{
	Camera(){
		impl = std::make_unique<OrthographicCamera>();
	}
	float* projection(float width,float height){
		mat4x4 m = impl->projection_matrix(width,height);
		static float res[16]; FOR(i,16) res[i] = m[i/4][i%4]; return res;
	}
	float* view(){
		mat4x4 m = impl->view_matrix();
		static float res[16]; FOR(i,16) res[i] = m[i/4][i%4]; return res;
	}

	void update(){
		impl->update();
	}

	std::unique_ptr<CameraInterface> impl;
};



// -------------------------------------------------------------------------------
//                                    CONTEXT
// -------------------------------------------------------------------------------
// => init almost everything
// => start and finish frames
// => give access to screen size
// => GLFW callbacks will be remove by refactoring into more than one C++ file


void framebuffer_size_callback(GLFWwindow* window,int width,int height);
void mouse_button_callback(GLFWwindow* window,int button,int action,int mods);
void cursor_position_callback(GLFWwindow* window,double mouseX,double mouseY);
void scroll_callback(GLFWwindow* window,double xOffset,double yOffset);

struct Context{

	void init(int w=1000,int h=1000){
		init_glfw(w,h);
		init_glad();
		init_imgui();
		init_mouse_call_backs();
	}
	~Context(){
		// Not sure I want to quit nicely... especially after the main
		//ImGui_ImplOpenGL3_Shutdown();
		//ImGui_ImplGlfw_Shutdown();
		//ImGui::DestroyContext();
		//glfwDestroyWindow(window);
		//glfwTerminate();
	}

	void init_mouse_call_backs(){
		glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
		glfwSetMouseButtonCallback(window,mouse_button_callback);
		glfwSetCursorPosCallback(window,cursor_position_callback);
		glfwSetScrollCallback(window,scroll_callback);
	}
	void init_glfw(int w,int h){
		um_assert(glfwInit());
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
		glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_DEPTH_BITS,24);
		window = glfwCreateWindow(1000,700,"Hexel",nullptr,nullptr);
		um_assert(window);

		glfwMakeContextCurrent(window);
		glfwSwapInterval(1);
	}
	void init_glad(){
		int version = gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress));
		um_assert(version != 0);
		Log::add(std::string("OpenGL version: ") + std::string((char*)glGetString(GL_VERSION)));
	}
	void init_imgui(){
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |=ImGuiConfigFlags_NavEnableKeyboard;
		ImGui::StyleColorsDark();
		um_assert(ImGui_ImplGlfw_InitForOpenGL(window,true));
		um_assert(ImGui_ImplOpenGL3_Init("#version 330"));
	}


	void begin_frame(){
		int width = 0;
		int height = 0;

		//Opengl
		glfwGetFramebufferSize(window,&width,&height);
		glViewport(0,0,width,height);
		glClearColor(0.05f,0.05f,0.08f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
	}
	void end_frame(){
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	std::pair<int,int> screen_size(){
		int h; int w;
		glfwGetFramebufferSize(window,&w,&h);
		return {w,h};
	} ;
	bool window_is_active(){ return !glfwWindowShouldClose(window); }
	GLFWwindow* window;
};








// -------------------------------------------------------------------------------
//                                    XCF
// -------------------------------------------------------------------------------
// => placeholder 

struct MultiMesh{
	template<class Mesh,class Attributes>
	struct MeshAttr{
		Mesh mesh;
		Attributes attributes;
	};


	PointSet points;
	NamedMap<MeshAttr<Triangles,SurfaceAttributes>> triangles;

	void load(std::string filename,bool connect = true){
		std::filesystem::path path(filename);
		std::string triname = path.stem().string();
		auto& new_elt = triangles.add(triname);
		Triangles& tri = new_elt.mesh;
		new_elt.attributes =read_by_extension(filename,new_elt.mesh);
		if(connect) tri.connect();
	}

};

struct XCF: public NamedMap<MultiMesh> {
	void load_multimesh(std::string filename,bool connect = true){
		std::string triname = std::filesystem::path(filename).stem().string();
		add(triname).load(filename,connect);
	}
};




// -------------------------------------------------------------------------------
//                                    ShaderManager
// -------------------------------------------------------------------------------
// => construct a dictonary of shaders 
// 


struct ShaderManager: public std::map<std::string,GLuint> {
	ShaderManager(){
	}
	GLuint compileShader(GLenum type,const char* source){
		GLuint shader = glCreateShader(type);
		glShaderSource(shader,1,&source,nullptr);
		glCompileShader(shader);
		GLint success = GL_FALSE;
		glGetShaderiv(shader,GL_COMPILE_STATUS,&success);
		if(!success){
			char log[1024] = {};
			glGetShaderInfoLog(shader,sizeof(log),nullptr,log);
			Log::error("shader compilation failed",log);
			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}
	GLuint createShaderProgram(const char* vertexSource,const char* fragmentSource){

		GLuint vertexShader =compileShader(GL_VERTEX_SHADER,vertexSource);
		if(!vertexShader)return 0;

		GLuint fragmentShader =compileShader(GL_FRAGMENT_SHADER,fragmentSource);
		if(!fragmentShader){ glDeleteShader(vertexShader); return 0; }


		GLuint program = glCreateProgram();
		glAttachShader(program,vertexShader);
		glAttachShader(program,fragmentShader);
		glLinkProgram(program);

		GLint success = GL_FALSE;
		glGetProgramiv(program,GL_LINK_STATUS,&success);

		if(!success){
			char log[1024] = {};
			glGetProgramInfoLog(program,sizeof(log),nullptr,log);
			Log::error("shader linking failed",log);
			glDeleteProgram(program);
			program = 0;
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return program;
	}
	std::string loadShader(const std::string& filename){
		std::ifstream file(filename);
		if(!file) Log::error("Cannot open shader: ",filename);
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}
	void add(std::string path,std::string shader_name){
		if(find(shader_name)!=end()) return;
		operator[](shader_name)=createShaderProgram(loadShader(path+shader_name+std::string(".vert")).c_str(),loadShader(path+shader_name+std::string(".frag")).c_str());
		um_assert(operator[](shader_name));
	}
};


// -------------------------------------------------------------------------------
//                                    RenderLayer + LayerManager
// -------------------------------------------------------------------------------
// => 
// 


struct RenderLayer{
	virtual void render()		=0;
	virtual void generate_gui(std::string name) =0;
	virtual bool resync_with_data()=0;
	virtual void render_primitive_id()				{Log::add("To be implemented");}
	virtual void render_constant_color(int layerid)	{Log::add("To be implemented"); }
	bool visible;
};

struct LayerManager: public NamedVector<RenderLayer> {
	void render(){
		FOR(i,size()) operator[](i).render();
	}
	void sync(){
		FOR(i,size()) if (!operator[](i).resync_with_data()){
			std::swap(collection[i],collection.back());
			collection.pop_back();
		};
	}
	
	void produce_picking_image(int* data,int w,int h){ Log::add("To be implemented"); }

};






// -------------------------------------------------------------------------------
//                                    Mouse + Keyboard States
// -------------------------------------------------------------------------------

struct MouseState{
	MouseState(){
		FOR(i,3) mouseDragging[i] = false;
		x = 0.0; y = 0.0; lastx = 0.0; lasty = 0.0;
		wheel_event_speed=0;
	}
	void set_wheel_event(double v){
		wheel_event_speed=v;
	}
	double get_wheel_event(bool consume=true){
		double ret = wheel_event_speed;
		if(consume) wheel_event_speed=0;
		return ret;
	}


	bool mouseDragging[3] ;
	double lastx,lasty,x,y;
	double wheel_event_speed;
};

struct KeyboardState{
	// adressed by ImGuiKey
	bool pressed(ImGuiKey key){ return data[key]; }

	void update();

	std::array<bool,2048> data;

};

// -------------------------------------------------------------------------------
//                                    Windows to explore things (layers/xcf/etc.)
// -------------------------------------------------------------------------------
// => are more or less independant of the mode
struct Window{
	virtual void generate_gui()=0;
};
struct WindowManager{
	NamedVector<Window> wins;
	void show_gui(){
		for(auto& [name,obj]:wins.collection) obj->generate_gui();
	}
};


// -------------------------------------------------------------------------------
//                                    Events
// -------------------------------------------------------------------------------
// we need some way to have objects interactions. Messages, listeners and callback would not simplify the conception
struct Event{
	enum {
		MOUSE_MOVED,
		MOUSE_PRESSED,
		MOUSE_RELEASED,
		KEY_PRESSED,
		KEY_RELEASED,
		MM_REMOVED,
		TRIANGLES_REMOVED,
		TRIANGLES_UPDATED
	} even_type;

	std::string object_name;// a string that allows to find the element
};

// -------------------------------------------------------------------------------
//                                    God: you know... he knows
// -------------------------------------------------------------------------------

namespace InteractionMode{ struct AbstractMode; }

struct God{
	// current interaction mode (controls everything: it is the entry point for different tools)
	static InteractionMode::AbstractMode* root_mode;

	// datas actually manipulated by the modeler	
	static XCF xcf;

	// input state
	static MouseState mouse;
	static KeyboardState keys;

	// event that occurred since last frame
	static std::vector<Event> events;

	// the camera
	static Camera camera;
	static LayerManager layers;							// layers to be combined into the final rendering

	static WindowManager win_manager;

	// API dependant 
	static Context context;
	static ShaderManager shaders;
};

XCF God::xcf;
LayerManager God::layers;
ShaderManager God::shaders;
InteractionMode::AbstractMode* God::root_mode;
MouseState God::mouse;
KeyboardState God::keys;
Context God::context;
Camera God::camera;
std::vector<Event> God::events;
WindowManager God::win_manager;






// ------------------------------------------------------------
// GLFW callbacks
// ------------------------------------------------------------

void framebuffer_size_callback(GLFWwindow* window,int width,int height){
	glViewport(0,0,width,height);
}

void mouse_button_callback(GLFWwindow* window,int button,int action,int mods){
	ImGui_ImplGlfw_MouseButtonCallback(window,button,action,mods);
	if(button <0 || button>2) { Log::error("Do not manage mouses with more than 3 buttons"); return; }
	God::mouse.mouseDragging[button] = (action == GLFW_PRESS);
	if (God::mouse.mouseDragging[button])
		God::events.push_back({Event::MOUSE_PRESSED,""});
	else 
		God::events.push_back({Event::MOUSE_RELEASED,""});
}

void cursor_position_callback(GLFWwindow* window,double mouseX,double mouseY){
	ImGui_ImplGlfw_CursorPosCallback(window,mouseX,mouseY);
	God::mouse.lastx=mouseX; std::swap(God::mouse.lastx,God::mouse.x);
	God::mouse.lasty=mouseY; std::swap(God::mouse.lasty,God::mouse.y);
}

void scroll_callback(GLFWwindow* window,double xOffset,double yOffset){
	ImGui_ImplGlfw_ScrollCallback(window,xOffset,yOffset);
	God::mouse.set_wheel_event(yOffset);
}







// ------------------------------------------------------------
//  drawback of everything a a .h file :(
// ------------------------------------------------------------

void OrthographicCamera::update(){
	double wheel = God::mouse.get_wheel_event();
	if(wheel!=0) {
		plop(wheel);
		zoom *= (1.+.1*wheel);
		zoom = std::clamp(zoom,.2,5.);
	}
	if(!God::mouse.mouseDragging[1]) return;
	if(!God::keys.pressed(ImGuiKey_LeftCtrl)) return;
	rotX += .01*(God::mouse.y-God::mouse.lasty); rotX = std::clamp(rotX,-M_PI,M_PI);
	rotY -= .01*(God::mouse.x-God::mouse.lastx); rotY = std::clamp(rotY,-M_PI/2.,M_PI/2.);
}


void KeyboardState::update(){
	for(int k=512; k<ImGuiKey_Oem102; k++){
		bool nv =ImGui::IsKeyDown(ImGuiKey(k));
		if(nv!=data[k]) {
			if(nv)
				God::events.push_back({Event::KEY_PRESSED,""});
			else
				God::events.push_back({Event::KEY_RELEASED,""});
			//plop(k); //====> run callbacks
		}
		data[k] =nv;
	}
}
