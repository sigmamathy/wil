#version 450

layout(location = 0) out vec4 oFrag;

layout(push_constant) uniform PushConstant {
	mat4 model;
	vec3 light_color;
} push;

void main() {
	oFrag = vec4(push.light_color, 1.f);
}
