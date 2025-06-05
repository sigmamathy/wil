#version 450

layout(location = 0) in vec2 ipos;

layout(binding = 0) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} umvp;

void main()
{
	gl_Position = umvp.proj * umvp.view * umvp.model * vec4(ipos, 0.f, 1.f);
	// gl_Position = umvp.view * vec4(ipos, 0, 1);
}

