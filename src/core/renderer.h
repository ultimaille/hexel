#pragma once
#include <glad/gl.h>

struct Renderer {
    GLuint vao,vbo;

    int npts;
    GLuint shaderProgram;
    float light_direction[3] = { 1,1,1 };
    int layer_id = -1;

    GLuint colormap;
    int texture_repeat=0;
    int texture_id=0;
    bool auto_range = true;
    float value_range[2] = { 0,0 };
    float texture_range[2] = { 0,0 };
    float color[3] = {.8,.8,.8};
    float color_map_prop=0;
    float ambient_prop=.5;

    struct Clipping {
        int mode = 1; // {0 = cell, 1 = std, 2 = slice}
        float normal[3] = {0,1,0};
        float pos[3] = {0.5,0.5,0.5};
        bool invert{false};
        bool enabled{false};
    } clipping;

    bool is_autorange = true;

    void generate_gui(std::string name);
    void shared_setup_before_rendering();

    // declare uniforms for raytraced primitives (sphere and cylinder)
    void declare_inv_projection_matrix();
    void declare_viewport();

    virtual void destroy();
};