#version 460 core

layout (location=0)
in vec3 v_normal;

layout (location=1)
in vec2 v_texcoord;

out vec4 f_col;

void main() {
   f_col = vec4(1.0, 1.0, 1.0, 1.0);
}
