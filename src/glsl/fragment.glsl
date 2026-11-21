#version 460 core

layout (location=0)
in vec2 v_texcoord;

layout (location=1)
in float v_shading;

layout (location=0)
out vec4 f_color;

uniform sampler2D u_atlas;

void main() {
    /* set 'f_color' */
    f_color = vec4(vec3(v_shading), 1.0);
}
