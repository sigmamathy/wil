#version 450

layout(location = 0) in vec2 vtex;

layout(location = 0) out vec4 ofrag;

layout(set = 1, binding = 0) uniform sampler2D utex;

void main()
{
	ofrag = texture(utex, vtex);
}
