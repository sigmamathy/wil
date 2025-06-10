#version 450

layout(location = 0) in vec3 ipos;
layout(location = 1) in vec2 itex;
layout(location = 2) in vec2 inormal;

layout(location = 0) out vec2 vtex;

layout(set = 0, binding = 0) uniform GlobalData {
    mat4 view;
    mat4 proj;
} uGlobal;

layout(push_constant) uniform PushConstant {
    mat4 model;
} push;

void main()
{
	gl_Position = uGlobal.proj * uGlobal.view * push.model * vec4(ipos, 1.f);
	vtex = itex;
}

