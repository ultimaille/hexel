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
        virtual void define_gui() = 0;
    };


    struct HexEdit : public AbstractMode{
        HexEdit(){
            God::root_mode = this;

            {

                God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "B0.step.mesh", true);
                //God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "mmB0", true);
                God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "B1.step.mesh", true);

                // normalize mesh
                BBox3 box;
                for (auto name : { 
                    //"mmB0",
                    "B1.step","B0.step" })
                {
                    Triangles& tri = God::xcf[name].triangles["triangles"].mesh;
                    for (auto v : tri.iter_vertices()) box.add(v.pos());
                }
                TrackBallCamera& cam = dynamic_cast<TrackBallCamera&>(*God::camera.impl);
                cam.pose.pivot = box.center();
                cam.projection.view_height = box.size()[1];

                HexEdit* root = static_cast<HexEdit*>(God::root_mode);

                God::layers.emplace_back<RenderLambertTriangles>("Lambert0").init(ObjectId({ chunk_xcf, "B0.step",chunk_triangles, "triangles" }));
                //God::layers.emplace_back<RenderLambertTriangles>("Lambert0bis").init(ObjectId({ chunk_xcf, "B0.step",chunk_triangles, "triangles" }));
                God::layers.emplace_back<RenderLambertTriangles>("Lambert1").init(ObjectId({ chunk_xcf, "B1.step",chunk_triangles, "triangles" }));
                //God::layers.emplace_back<SSAO>("SSAO").init();
                //God::layers.emplace_back<RenderSpheres>("RenderSpheres").init(ObjectId({ chunk_xcf, "B0.step",chunk_pointset }));
                //God::layers.emplace_back<RenderTubes>("RenderTubes").init(ObjectId({ chunk_xcf, "B0.step",chunk_polylines , "polylines" }));

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
                    MultiMesh mm;
                    std::vector<std::chrono::steady_clock::time_point> t;
                    t.push_back(std::chrono::high_resolution_clock::now());

                    std::string tmp = "C:\\NICOTMP\\";
                    //mm.load_geogram(std::string(TEST_INPUT_DIR) + "B0.step.mesh");
                    mm.load_geogram(tmp+ "B0.geogram");

                    t.push_back(std::chrono::high_resolution_clock::now());
                    mm.save_to_path(tmp + "test_compressed");
                    t.push_back(std::chrono::high_resolution_clock::now());
                    mm.save_to_path(tmp + "test_after_save_compressed", true);
                    t.push_back(std::chrono::high_resolution_clock::now());

                    MultiMesh mm1;
                    t.push_back(std::chrono::high_resolution_clock::now());
                    mm1.load_from_path(tmp + "test_compressed");
                    t.push_back(std::chrono::high_resolution_clock::now());
                    mm1.save_to_path(tmp + "test_after_load_compressed", true);

                    FOR(i, t.size() - 1) std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(t[i + 1] - t[i]) << std::endl;
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
