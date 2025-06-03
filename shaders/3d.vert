#version 450

layout(location=0) in vec2 ipos;

void main()
{
	gl_Position = vec4(ipos, 0.f, 1.f);
}

