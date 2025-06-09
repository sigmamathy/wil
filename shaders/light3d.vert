#version 450

layout(location = 0) in vec3 iPos;

layout(set = 0, binding = 0) uniform GlobalData {
	mat4 view;
    mat4 proj;
} uGlobal;

layout(push_constant) uniform PushConstant {
	mat4 model;
	vec4 light_color;
} push;

void main() {
	gl_Position = uGlobal.proj * uGlobal.view * push.model * vec4(iPos, 1.f);
}
