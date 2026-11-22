#version 460 core

layout (location=0)
in uint a_vertex;


layout (location=0)
out vec2 v_texcoord;


layout (location=1)
out float v_shading;


uniform mat4 u_model;
uniform mat4 u_proj;
uniform mat4 u_view;


void main() {
    /* unpack: 'position' (18-bit) */
    uint x = (a_vertex >>  0) & 0x3fu,
         y = (a_vertex >>  6) & 0x3fu,
         z = (a_vertex >> 12) & 0x3fu;

    /* unpack: 'tex_index' (6-bit) */
    uint tex_index = (a_vertex >> 18) & 0x3fu;

    /* unpack: 'tex_corner' (2-bit) */
    uint tex_corner = (a_vertex >> 24) & 0x3u;
    
    /* unpack: 'direction' (2-bit) */
    uint direction = (a_vertex >> 26) & 0x3u;
    
    /* set 'gl_Position' */
    gl_Position = u_proj  *
                  u_view  *
                  u_model *
                  vec4(float(x), float(y), float(z), 1.0);

    /* get 'texcoord' */
    uint x_idx = tex_index % uint(16),
         y_idx = tex_index / uint(16);
    vec2 corner = vec2((tex_corner >> 1) & 0x1u,
                       (tex_corner >> 0) & 0x1u);

    /* set 'v_texcoord' */
    v_texcoord = (vec2(float(x_idx),
                       float(y_idx)) + corner) / 16.0;

    /* set 'v_shading' */
    /* this will probably suck but whatever */
    switch (direction) {
        case (0): { v_shading = 0.60; } break;
        case (1): { v_shading = 0.85; } break;
        case (2): { v_shading = 0.75; } break;
        case (3): { v_shading = 1.00; } break;
    }
}
