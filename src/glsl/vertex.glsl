#version 460 core

layout (location=0)
in uint a_vertex;


layout (location=0)
out vec2 v_texcoord;


layout (location=1)
out vec3 v_normal;


layout (location=2)
out float v_shading;


uniform mat4 u_model;
uniform mat4 u_proj;
uniform mat4 u_view;
    
const uvec2 BLOCK_SIZE = uvec2(16, 16);
const uvec2 ATLAS_SIZE = uvec2(256, 256);


void main() {
    /* unpack: 'position' (18-bit) */
    uint x = (a_vertex >>  0) & 0x3fu,
         y = (a_vertex >>  6) & 0x3fu,
         z = (a_vertex >> 12) & 0x3fu;

    /* unpack: 'tex_index' (6-bit) */
    uint tex_index = (a_vertex >> 18) & 0x3fu;

    /* unpack: 'tex_corner' (2-bit) */
    uint tex_corner = (a_vertex >> 24) & 0x3u;
    
    /* unpack: 'face' (3-bit)
     * NOTE:
     *  'face' must match 'enum face_e' values
     * */
    uint face = (a_vertex >> 26) & 0x7u;
    
    /* set 'gl_Position' */
    gl_Position = u_proj  *
                  u_view  *
                  u_model *
                  vec4(float(x), float(y), float(z), 1.0);

    /* get 'texcoord' */
    vec2 res = ATLAS_SIZE/ BLOCK_SIZE;
    uint x_idx = tex_index % uint(res.x),
         y_idx = tex_index / uint(res.y);
    vec2 corner = vec2((tex_corner >> 1) & 0x1u,
                       (tex_corner >> 0) & 0x1u);

    /* set 'v_texcoord' */
    v_texcoord = (vec2(float(x_idx), float(res.y - 1 - y_idx)) + corner) / res;

    /* set 'v_normal' and 'v_shading' */
    switch (face) {
        /* face : NORTH */
        case (0): { v_shading = 0.90; 
                    v_normal = vec3(0.0, 0.0, -1.0); } break;
        
        /* face : SOUTH */
        case (1): { v_shading = 0.90; 
                    v_normal = vec3(0.0, 0.0, 1.0); } break;
        
        /* face : WEST */
        case (2): { v_shading = 0.80; 
                    v_normal = vec3(-1.0, 0.0, 0.0); } break;
        
        /* face : EAST */
        case (3): { v_shading = 0.80; 
                    v_normal = vec3(1.0, 0.0, 0.0); } break;
        
        /* face : DOWN */
        case (4): { v_shading = 0.70; 
                    v_normal = vec3(0.0, -1.0, 0.0); } break;
        
        /* face : TOP */
        case (5): { v_shading = 1.00; 
                    v_normal = vec3(0.0, 1.0, 0.0); } break;
    }
}
