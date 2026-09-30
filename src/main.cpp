#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <ultimaille/all.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cstdlib>
#include <iostream>

#include <string>

#include <ultimaille/all.h>
#include "core/core.h"

#include "panels/xcf_explorer.h"
#include "panels/layer_explorer.h"

#include "layers/basic_layers.h"
#include "layers/ssao.h"

// -------------------------------------------------------------------------------
//                                    Modes to define the behavior of a specific application
// -------------------------------------------------------------------------------
// ==> not satisfying  ATM

namespace InteractionMode{
	struct AbstractMode{
		virtual void define_gui()=0;
	};


	struct HexEdit: public AbstractMode{
		HexEdit(){
			God::root_mode = this;

			{

				God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "B0.step.mesh", true);
				God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "B1.step.mesh", true);

				// normalize mesh
				BBox3 box;
				for (auto name : { "B0.step","B1.step" })
				{
					Triangles& tri = God::xcf[name].triangles["triangles"].mesh;
					for (auto v : tri.iter_vertices()) box.add(v.pos());
				}
				TrackBallCamera& cam = dynamic_cast<TrackBallCamera&>(*God::camera.impl);
				cam.pose.pivot = box.center();
				cam.projection.view_height = box.size()[1];

				HexEdit* root = static_cast<HexEdit*>(God::root_mode);

				God::layers.emplace_back<RenderLambertTriangles>("Lambert0").init(ObjectId({ chunk_xcf, "B0.step",chunk_triangles, "triangles" }));
				God::layers.emplace_back<RenderLambertTriangles>("Lambert0bis").init(ObjectId({ chunk_xcf, "B0.step",chunk_triangles, "triangles" }));
				God::layers.emplace_back<RenderLambertTriangles>("Lambert1").init(ObjectId({ chunk_xcf, "B1.step",chunk_triangles, "triangles" }));
				God::layers.emplace_back<SSAO>("SSAO").init();
				God::layers.emplace_back<RenderSpheres>("RenderSpheres").init(ObjectId({ chunk_xcf, "B0.step",chunk_pointset }));
				God::layers.emplace_back<RenderTubes>("RenderTubes").init(ObjectId({ chunk_xcf, "B0.step",chunk_polylines , "polylines" }));
				std::swap(God::layers.items[0],God::layers.items[1]);

			}
		}

		void define_gui() {
			ImGui::Begin("ModeWindow", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
			{
				ImGui::Text("ModeWindow");
				ImGui::Separator();
				static char str0[128] = "Hello, world!";
				ImGui::InputText("input text", str0, 128);
				if (ImGui::Button("Create MultiMesh", ImVec2(180, 40))) {
					Log::add("Button pressed");
				}
			}

			ImGui::End();
		}
	};




};






int main() {
    God::context.init();
    InteractionMode::HexEdit look;
    God::panels.emplace_back<XCFExplorer>("xcf_window");
    God::panels.emplace_back<LayerExplorer>("layer_window");
    while(God::context.window_is_active()){
        God::mouse.wheel_speed = 0;
        God::mouse.previous = God::mouse.current;
        glfwPollEvents();
        if(!ImGui::GetIO().WantCaptureKeyboard || !ImGui::GetIO().WantCaptureMouse)
            God::keys.update();
        God::events.dispatch();

        God::context.begin_frame();
        God::layers.render();
        God::panels.show_gui();
        God::root_mode->define_gui();
        God::context.end_frame();
    }
    God::layers.destroy();
    God::context.destroy();
    return EXIT_SUCCESS;
}
