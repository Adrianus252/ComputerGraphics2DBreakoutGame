
#version 460 core

// vertex shader for rendering 2D objects

layout (location = 0) in vec2 in_pos; // vertex position input (mesh data)
layout (location = 2) in vec2 in_uv; // UV coordinates input (mesh data)

layout (location = 0) out vec2 out_uv; // output UV coordinates to the fragment shader

layout (location = 0) uniform vec2 u_pos; // uniform variable for position 
layout (location = 1) uniform vec2 u_size; // uniform variable for size
layout (location = 2) uniform vec2 u_screen; // uniform variable for screen dimensions 

void main() {
    vec2 world_pos = u_pos + in_pos * u_size; // scale and position the vertex in world space
    vec2 ndc = world_pos / u_screen * 2.0 - 1.0; // convert to normalized device coordinates (NDC)
    ndc.y = -ndc.y; // flip Y axis for correct orientation in screen space
    out_uv = in_uv; // texure coordinates are passed through to the fragment shader
    gl_Position = vec4(ndc, 0.0, 1.0); // position output for the vertex shader, with z=0 and w=1 for 2D rendering
}

