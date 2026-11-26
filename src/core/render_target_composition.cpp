#include "render_target_composition.h"
#include <set>
#include <algorithm>

void RenderTargetComposition::init(int w, int h) {
    God::shaders.create_if_needed(std::string(SHADERS_DIR), "blend");
    composite.init(w, h);
    initialize_quad();
    width = w;
    height = h;
}

void RenderTargetComposition::blend(GLuint a, GLuint b, Group::BlendOp blend_op) {
    // Use blend shader
    GLuint blend_program = God::shaders["blend"];
    glUseProgram(blend_program);

    glUniform1i(glGetUniformLocation(blend_program, "mix"), static_cast<int>(blend_op));

    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, a);
    glUniform1i(glGetUniformLocation(blend_program, "tex_a"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, b);
    glUniform1i(glGetUniformLocation(blend_program, "tex_b"), 1);

    draw_quad();
}

void RenderTargetComposition::begin() {
    auto has_visible_layer = [](const Group &group) {
        if (!group.visible) return false;
        for (Layer &layer : God::layers.from_group(group.id))
            if (layer.visible) return true;
        return false;
    };

    // which groups need their own render target this frame
    std::set<int> needed;
    int owner = -1;
    for (int i = 0; i < God::layers.groups.size(); ++i) {
        auto &group = God::layers.groups[i];
        if (i == 0 || group.blend_op != Group::BlendOp::NONE)
            owner = group.id;
        if (has_visible_layer(group))
            needed.insert(owner);
    }

    // the first group draws straight into the composite when it does not bleny to avoid an extra copy
    direct_group = -1;
    if (!God::layers.groups.empty()) {
        auto &first = God::layers.groups[0];
        if (first.blend_op == Group::BlendOp::NONE && needed.contains(first.id)) {
            direct_group = first.id;
            needed.erase(first.id);
        }
    }

    // drop targets that are no longer needed (group deleted or blend op set to NONE)
    std::erase_if(targets, [&](const auto &p) { return !needed.contains(p.first); });

    // create missing targets, resize stale ones
    for (int id : needed) {
        auto [it, inserted] = targets.try_emplace(id);
        RenderTarget &t = it->second;
        if (inserted)
            t.init(width, height);
        else
            if (t.width != width || t.height != height)
                t.resize(width, height);
    }
}

void RenderTargetComposition::bind(int group_id) {
    // Only bind framebuffer if the target exists for group
    // This behavior implicitly don't bind if there is no blending,
    // enabling drawing in the same render target
    if (group_id == direct_group) {
        composite.clear(background_color[0], background_color[1], background_color[2], background_color[3]); // clear & bind framebuffer
        cur_target = group_id;
    } else if (targets.contains(group_id)) {
        targets[group_id].clear(background_color[0], background_color[1], background_color[2], background_color[3]); // clear & bind framebuffer
        cur_target = group_id;
    }
}

void RenderTargetComposition::render_groups() {
    for (auto &group : God::layers.groups) {
        bind(group.id);
        God::layers.render(group.id);
    }
}

void RenderTargetComposition::render_ids() {
    if (targets.empty() && direct_group == -1)
        return;
    for (auto &[id, t] : targets)
        t.draw_ids(true);
    if (direct_group != -1)
        composite.draw_ids(true);
    render_groups();
    for (auto &[id, t] : targets)
        t.draw_ids(false);
    if (direct_group != -1)
        composite.draw_ids(false);
}

void RenderTargetComposition::end() {
    // The composite is cleared exactly once per frame: in bind() when the direct group draws into it, here otherwise
    if (direct_group == -1)
        composite.clear(background_color[0], background_color[1], background_color[2], background_color[3]);

    if (targets.empty()) return;

    // Merge all color attachments into main render target, keeping its depth
    glBindFramebuffer(GL_FRAMEBUFFER, composite.framebuffer);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);  // Don't write to depth buffer
    glDepthFunc(GL_ALWAYS); // Pass all depth tests (use existing depth)

    // Render all framebuffer colors as layers
    for (int i = 0; i < God::layers.groups.size(); ++i) {
        auto &group = God::layers.groups[i];
            if (targets.contains(group.id))
                blend(composite.color, targets[group.id].color, group.blend_op);
    }

    // Re-enable depth writes
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
}

void RenderTargetComposition::destroy() {

    targets.clear();
    composite.destroy();

    if (quad_vbo != 0) {
        glDeleteBuffers(1, &quad_vbo);
        quad_vbo = 0;
    }

    if (quad_vao != 0) {
        glDeleteVertexArrays(1, &quad_vao);
        quad_vao = 0;
    }
}

