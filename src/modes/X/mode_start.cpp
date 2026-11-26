#include <modes/X/mode_start.h>
#include <modes/X/root_mode.h>

namespace ModeX{
    StartingMode::StartingMode(RootMode& mode) : AbstractMode(mode){}
    void StartingMode::define_gui() {

        if (ImGui::Button("Load/Import From CAD")) ImGui::OpenPopup("load_CAD_file");
        if (FilePopup("load_CAD_file", std::filesystem::path(mode.CAD_file).parent_path().string(), mode.CAD_file, { ".geogram",".meshb",".mesh" }))
            if (!mode.CAD_file.empty()) {
                //scene().reset_xcf();
                mode.project_directory = mode.working_directory + std::filesystem::path(mode.CAD_file).stem().string();
                if (!std::filesystem::exists(mode.project_directory)){
                    Log::info(mode.project_directory+" doesn't exist, need to import file " + mode.CAD_file);
                    std::filesystem::create_directory(mode.project_directory);
                    import_new_project_from_CAD(mode.CAD_file);
                }
                Log::info("Loading project " + mode.CAD_file);
                scene().load_project(mode.project_directory);
                

                desactivate();
                mode.active_mode_id = 1;
                mode.sub_mode_basic_edit.activate();


            }
    }
    void StartingMode::import_new_project_from_CAD(std::string filename){
        const std::filesystem::path project_dir{ mode.project_directory };
        std::filesystem::create_directory(project_dir);

        BBox3 bbox;
        {   // import the CAD
            MultiMesh mm;
            mm.load_geogram(filename, false);
            FOR(v, mm.pointset.size()) bbox.add(mm.pointset[v]);

            PointAttribute<int> touch_lock("lock", mm.pointset_attributes, mm.pointset, 0);


            if (mm.polylines.empty()) mm.polylines.try_emplace("");
            auto& pl = mm.polylines[""];
            pl.attributes.points.clear();


            std::vector<int> gmsh_import_feature(pl.mesh.nedges());
            {
                EdgeAttribute<int> region("region", pl.attributes, pl.mesh);
                for (auto e : pl.mesh.iter_edges()) gmsh_import_feature[e] = region[e];
            }
            pl.attributes.edges.clear();
            EdgeAttribute<int> feat_id("feat_id", pl.attributes, pl.mesh);
            for (auto e : pl.mesh.iter_edges()) feat_id[e] = gmsh_import_feature[e];

            {// create opposite edges
                pl.mesh.disconnect();
                int nedges = pl.mesh.nedges();
                pl.mesh.create_edges(pl.mesh.nedges());
                FOR(e, nedges){
                    FOR(lv, 2)pl.mesh.vert(e + nedges, lv) = pl.mesh.vert(e, 1 - lv);
                    feat_id[e + nedges] = feat_id[e];
                }
            }
            pl.mesh.connect();
            for (auto v : pl.mesh.iter_vertices()) for (auto e0 : v.iter_edges())for (auto e1 : v.iter_edges())
                if (feat_id[e0] != feat_id[e1]) touch_lock[v] = 1;


            if (mm.triangles.empty()) mm.triangles.try_emplace("");
            auto& tri = mm.triangles[""];
            tri.attributes.points.clear();
            tri.attributes.corners.clear();
            tri.attributes.facets.clear();
            FacetAttribute<int> region("region_id", tri.attributes, tri.mesh);
            for (auto f : tri.mesh.iter_facets()) region[f] = f;

            mm.quads.clear(); mm.polygons.clear(); mm.hexahedra.clear(); mm.pyramids.clear(); mm.wedges.clear();
            mm.save_to_path((project_dir / "geom").string(), true);
        }

        { // create an empty mesh
            MultiMesh mm;
            mm.hexahedra.try_emplace("");
            CellAttribute<int> block_id("block_id", mm.hexahedra[""].attributes, mm.hexahedra[""].mesh);

            Hexahedra& hex = mm.hexahedra[""].mesh;
            mm.pointset.create_points(12);
            hex.create_cells(2);
            FOR(k, 3)FOR(j, 2)FOR(i, 2)
                mm.pointset[i + 2 * j + 4 * k] = bbox.min + vec3(i * bbox.size()[0], j * bbox.size()[1], k * bbox.size()[2] / 2.);
            FOR(v, 8) hex.vert(0, v) = v;
            FOR(v, 8) hex.vert(1, v) = 4 + v;
            FOR(c, 2) block_id[c] = c;

            mm.save_to_path((project_dir / "mesh").string(), true);
        }
    }
    void StartingMode::handle(Event event) {
        Mode::handle(event);
    }
};