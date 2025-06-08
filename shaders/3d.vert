#version 450

layout(location = 0) in vec3 ipos;
layout(location = 1) in vec2 itex;

layout(location = 0) out vec2 vtex;

layout(set = 0, binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} umvp;

void main()
{
	gl_Position = umvp.proj * umvp.view * umvp.model * vec4(ipos, 1.f);
	vtex = itex;
}

