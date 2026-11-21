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
    /* unpack: 'position' */
    uint x = (a_vertex >>  0) & 0x3fu,
         y = (a_vertex >>  6) & 0x3fu,
         z = (a_vertex >> 12) & 0x3fu;
    vec3 position = vec3(float(x),
                         float(y),
                         float(z));

    /* unpack: 'texcoord' */
    uint t = (a_vertex >> 18) & 0x3fu;
    uint ix = t % 16,
         iy = t / 16;
    vec2 texcoord = vec2(float(ix) / 16.0,
                         float(iy) / 16.0);
    
    /* unpack: 'direction' */
    uint d = (a_vertex >> 24) & 0x3u;
    /* this will probably suck but whatever */
    float shading = 0.0;
    switch (d) {
        case (0): { shading = 1.00; } break;
        case (1): { shading = 0.90; } break;
        case (2): { shading = 0.85; } break;
        case (3): { shading = 0.75; } break;
    }
    
    /* set 'gl_Position' */
    gl_Position = u_proj  *
                  u_view  *
                  u_model *
                  vec4(position, 1.0);

    /* set 'v_texcoord' */
    v_texcoord = texcoord;

    /* set 'v_shading' */
    v_shading = shading;
}
