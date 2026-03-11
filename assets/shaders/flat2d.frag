// fragment shader for rendering 2D objects
// If texturing is disabled, it outputs the uniform color. 
#version 460 core

layout (location = 0) in vec2 out_uv; // UV coordinates passed from the vertex shader 
layout (location = 0) out vec4 out_color; // output color of the fragment

layout (location = 3) uniform vec4 u_color; // uniform color for the fragment
layout (location = 4) uniform int u_use_texture; // uniform flag for texture usage
layout (binding = 0) uniform sampler2D u_texture;

void main() {
    vec4 tex_color = texture(u_texture, out_uv); // sample the texture color using UV coordinates
    out_color = (u_use_texture == 1) ? (tex_color * u_color) : u_color; 
}
