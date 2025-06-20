#version 450

layout(location = 0) in vec3 iPos;
layout(location = 1) in vec2 iTexCoord;
layout(location = 2) in vec3 iNormal;

layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec3 vFragPos;  
layout(location = 2) out vec3 vNormal;

layout(set = 0, binding = 0) uniform GlobalData {
    mat4 view;
    mat4 proj;
	vec3 viewPos;
} uGlobal;

layout(push_constant) uniform PushConstant {
    mat4 model;
	bool has_texture;
} push;

void main()
{
	gl_Position = uGlobal.proj * uGlobal.view * push.model * vec4(iPos, 1.f);
	vTexCoord = iTexCoord;
	vFragPos = vec3(push.model * vec4(iPos, 1.f));
	vNormal = mat3(transpose(inverse(push.model))) * iNormal;
}

