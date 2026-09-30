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
#include <imgui.h>
#include <filesystem>
#include <string>
namespace fs = std::filesystem;

bool FilePopup(const char* id, std::string& out, bool dir = false, const char* ext = nullptr)
{
    static fs::path path, sel;
    static bool init = false;
    bool done = false;

    if (!ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        return false;

    if (!init){ path = fs::current_path(); sel.clear(); init = true; }

    ImGui::TextUnformatted(path.string().c_str());

    if (ImGui::Button("[..]") && path.has_parent_path())
        path = path.parent_path(), sel.clear();

    ImGui::BeginChild("files", { 400,250 }, true);

    for (auto& e : fs::directory_iterator(path)){
        auto p = e.path(); bool d = e.is_directory();
        if (!d && ext && p.extension() != ext)continue;

        std::string n = (d ? "[D] " : "") + p.filename().string();
        ImGui::Selectable(n.c_str(), p == sel);

        if (ImGui::IsItemClicked()) sel = p;

        if (d && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            path = p, sel.clear();

        if (!d && !dir && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
            out = p.string(), done = true;
    }

    ImGui::EndChild();

    if (ImGui::Button(dir ? "Select" : "Open") &&
        ((dir && fs::is_directory(sel)) || (!dir && fs::is_regular_file(sel))))
        out = sel.string(), done = true;

    ImGui::SameLine();
    if (ImGui::Button("Cancel"))done = true;

    if (done)ImGui::CloseCurrentPopup();
    ImGui::EndPopup();
    return done;
}

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

                God::xcf.load_multimesh(std::string(TEST_INPUT_DIR) + "mmB0", true);

                // normalize mesh
                BBox3 box;
                Triangles& tri = God::xcf["mmB0"].triangles["triangles"].mesh;
                for (auto v : tri.iter_vertices()) box.add(v.pos());
                
                TrackBallCamera& cam = dynamic_cast<TrackBallCamera&>(*God::camera.impl);
                cam.pose.pivot = box.center();
                cam.projection.view_height = box.size()[1];

                HexEdit* root = static_cast<HexEdit*>(God::root_mode);
                God::layers.emplace_back<RenderLambertTriangles>("Lambert2").init(ObjectId({ chunk_xcf, "mmB0",chunk_triangles, "triangles" }));
                God::layers.emplace_back<RenderSpheres>("RenderSpheres").init(ObjectId({ chunk_xcf, "mmB0",chunk_pointset }));
                God::layers.emplace_back<RenderTubes>("RenderTubes").init(ObjectId({ chunk_xcf, "mmB0",chunk_polylines , "polylines" }));
                God::layers.emplace_back<SSAO>("SSAO").init();

            }
        }

        void define_gui() {
            ImGui::Begin("ModeWindow", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            {
                ImGui::Text("ModeWindow");
                ImGui::Separator();
                static char str0[128] = "Hello, world!";
                ImGui::InputText("input text", str0, 128);


                static std::string path;

                if (ImGui::Button("Browse"))
                    ImGui::OpenPopup("FilePopup");


                if (FilePopup("FilePopup", path)){
                    auto filename = fs::path(path).filename().string();
                    plop(filename);
                }
                //FilePopup("FilePopup", path, true);


                if (ImGui::Button("Create MultiMesh", ImVec2(180, 40))) {
                    Log::add("Button pressed");

                    //MultiMesh mm;
                    //std::vector<std::chrono::steady_clock::time_point> t;
                    //t.push_back(std::chrono::high_resolution_clock::now());
                    //std::string tmp = "C:\\NICOTMP\\";
                    ////mm.load_geogram(std::string(TEST_INPUT_DIR) + "B0.step.mesh");
                    //mm.load_geogram(tmp+ "B0.geogram");

                    //t.push_back(std::chrono::high_resolution_clock::now());
                    //mm.save_to_path(tmp + "test_compressed");
                    //t.push_back(std::chrono::high_resolution_clock::now());
                    //mm.save_to_path(tmp + "test_after_save_compressed", true);
                    //t.push_back(std::chrono::high_resolution_clock::now());

                    //MultiMesh mm1;
                    //t.push_back(std::chrono::high_resolution_clock::now());
                    //mm1.load_from_path(tmp + "test_compressed");
                    //t.push_back(std::chrono::high_resolution_clock::now());
                    //mm1.save_to_path(tmp + "test_after_load_compressed", true);
                    //FOR(i, t.size() - 1) std::cerr << std::chrono::duration_cast<std::chrono::milliseconds>(t[i + 1] - t[i]) << std::endl;
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
