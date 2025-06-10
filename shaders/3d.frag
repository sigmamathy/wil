#version 450

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec3 vFragPos;  
layout(location = 2) in vec3 vNormal;

layout(location = 0) out vec4 oFragColor;

layout(set = 0, binding = 1) uniform Light {
	vec3 pos;
} uLight;

layout(set = 1, binding = 0) uniform sampler2D uTexSampler;

const float ambient_strength = 0.02f;

void main()
{
	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(uLight.pos - vFragPos);  

	vec3 diffuse = max(dot(norm, lightDir), 0.0) * vec3(1.f,1.f,1.f);
	vec3 ambient = ambient_strength * vec3(1.f, 1.f, 1.f);

	oFragColor = vec4(diffuse + ambient, 1.f) * texture(uTexSampler, vTexCoord);
}
