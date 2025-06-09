#version 450

layout(push_constant) uniform PushConstant {
	mat4 model;
	vec4 light_color;
} push;

void main() {
	oFrag = vec4();
}
