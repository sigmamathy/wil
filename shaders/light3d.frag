#version 450

layout(location = 0) out vec4 oFrag;

layout(push_constant) uniform PushConstant {
	mat4 model;
	vec4 light_color;
} push;

void main() {
	oFrag = push.light_color;
}
