#pragma once
#include <misc/purgatory.h>
#include <layers/ssao.h>

struct PickFacetInteractor : public Interactor {

    // Note: put this function somewhere else (in camera for example ?) can be useful
    vec2 proj(vec3 a) {
        const int w = God::context.render_target.width;
        const int h = God::context.render_target.height;

        auto p = God::camera.projection_matrix() * God::camera.view_matrix() * vec4(a, 1.);
        auto ndc = vec2(p[0], p[1]) / p[3];
        vec2 scr(w * (ndc.x + 1.0f) * 0.5f, h * (1.0f - ndc.y) * 0.5f);
        return scr;
    }

    void on_press(int button, vec2 p) override {
        // Note: here possible to use delegate pattern in order to check key_state_is_ok on Interator
        if (button != 0 || !key_state_is_OK()) return;

        Picker picker;
        auto pr = picker.at(p);

        if (pr.mesh_id != hex_path) return;

        // Picking
        facet_id = pr.filtered_primitive_id(hex_path, AttributeBase::CELLFACETS);
        std::cout << "facet id: " << facet_id << std::endl;

        if (facet_id < 0) return;

        Hexahedra &hex = hex_path;
        VolumeAttributes &attr = hex_path;
        
        // Attribute display
        // Note: here no possible to bind unammed attribute...
        const std::string attr_name = "selection";
        CellFacetAttribute<bool> selection(attr_name, attr, hex, false);
        selection.fill(false);
        selection[facet_id] = true;
        God::layers[layer_name].bind_attribute(root.xcf[hex_path.parent().name()].hexahedra[hex_path.name()].attr_facets[attr_name]);

        Quad3 facet_quad = hex.facet(facet_id);
        vec3 a = pr.point();
        // project
        vec2 scr_a = proj(a);
        vec2 scr_b = proj(a + facet_quad.normal()); // Assume normal is normalized (length=1)

        vec2 projected_normal = scr_b - scr_a;
        pixels_per_unit = projected_normal.norm();

        if (pixels_per_unit > 1e-6f)
            projected_normal_in_pixels = projected_normal / pixels_per_unit;
        
    }

    void on_drag(int button, vec2 a, vec2 b) override {
        // Note: here possible to use delegate pattern in order to check key_state_is_ok on Interator
        if (button != 0 || !key_state_is_OK() ||  facet_id < 0 || pixels_per_unit < 1e-6f) return;

        // Signed mouse movement along the projected face normal, in pixels.
        double pixel_delta = (b - a) * projected_normal_in_pixels;
        // Convert pixels to world units along the face normal.
        double world_delta = pixel_delta / pixels_per_unit;

        Hexahedra &hex = hex_path;
        auto f = hex.facet(facet_id);
        Quad3 facet_quad = hex.facet(facet_id);
        vec3 displacement = world_delta * facet_quad.normal();

        for (int lv = 0; lv < 4; ++lv) {
            auto v = f.vertex(lv);
            v.pos() += displacement;
        }

        hex_path.broadcast(EventTypes::UPDATED);
    }

    void on_release(int button, vec2 p) override {
        facet_id = -1;
    }


    std::string layer_name;

    MeshPath hex_path;
    // MultiMeshPath::Volumes::Volume hex_path;
    int facet_id = -1;
    vec2 projected_normal_in_pixels;
    double pixels_per_unit;
};

struct CoreTestMode : public Mode {

    InteractorTrackBallCamera camera_interactor;
    PickFacetInteractor pick_facet_interactor;

    CoreTestMode() {
        camera_interactor.ctrl_must_be_down = true;
        pick_facet_interactor.ctrl_must_be_up = true;
        interactors.push_back(&camera_interactor);
        interactors.push_back(&pick_facet_interactor);
    }

    void create_box() {
        God::xcf.add("box");
        
        // Note: add_mesh should be splitted into multiple overloads
        // God::xcf.add_mesh(hex_path); 
        // auto hex_path = God::xcf.add_mesh<Hexahedra>("box", "hex");
        // Better, does not change nothing in term of code quantity, but 
        // no need to construct path before (that can be unvalid), function return valid path & pointer to new hex
        auto hex_path = God::xcf.add_hexahedra("box", "hex");

        // Create box
        Hexahedra &hex = hex_path;
        hex.points.create_points(8);
        hex.create_cells(1);
        hex.points[0] = {-0.5,-0.5,-0.5};
        hex.points[1] = {0.5,-0.5,-0.5};
        hex.points[2] = {-0.5,0.5,-0.5};
        hex.points[3] = {0.5,0.5,-0.5};
        hex.points[4] = {-0.5,-0.5,0.5};
        hex.points[5] = {0.5,-0.5,0.5};
        hex.points[6] = {-0.5,0.5,0.5};
        hex.points[7] = {0.5,0.5,0.5};
        hex.vert(0, 0) = 0;
        hex.vert(0, 1) = 1;
        hex.vert(0, 2) = 2;
        hex.vert(0, 3) = 3;
        hex.vert(0, 4) = 4;
        hex.vert(0, 5) = 5;
        hex.vert(0, 6) = 6;
        hex.vert(0, 7) = 7;

        // Note: here too complicated. 
        // We must: add group, add layer, init layer & passing mesh path
        // Maybe we just want to do something like
        // God::layers.add_init("box", hex_path); => 
        // - create a new group when not specified
        // - layer is type of default layer according to mesh type
        // - init is made automatically
        // 
        int group_id = God::layers.add_group("box_group");
        auto &hex_layer = God::layers.add<HexahedraLayer>("box", group_id); // TODO here get default layer for hex ?
        hex_layer.init(hex_path);

        // Configure layer
        hex_layer.shrink = 0.f; // No shrink

        hex_path.broadcast(EventTypes::UPDATED);

        pick_facet_interactor.hex_path = hex_path;
        pick_facet_interactor.layer_name = "box";
    }

    void define_gui() {
        // Note: use delegate pattern here to avoid declaring ImGui::Begin("Mode") each time
        ImGui::Begin("Mode", nullptr);
        if (ImGui::Button("Create box")) {
            create_box();
        }
        ImGui::End();
    }
};