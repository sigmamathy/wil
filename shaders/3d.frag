#version 450

layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec3 vFragPos;  
layout(location = 2) in vec3 vNormal;

layout(location = 0) out vec4 oFragColor;

layout(set = 0, binding = 0) uniform GlobalData {
    mat4 view;
    mat4 proj;
	vec3 viewPos;
} uGlobal;

layout(set = 0, binding = 1) uniform Light {
	vec3 pos;
	vec3 color;
} uLight;

layout(set = 1, binding = 0) uniform sampler2D uTexSampler;

const float ambient_strength = 0.02f;
const float specular_strength = 0.5f;

void main()
{
	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(uLight.pos - vFragPos);  

	vec3 diffuse = max(dot(norm, lightDir), 0.0) * uLight.color;
	vec3 ambient = ambient_strength * uLight.color;

	vec3 viewDir = normalize(uGlobal.viewPos - vFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specular_strength * spec * uLight.color;  

	oFragColor = vec4(diffuse + ambient + specular, 1.f) * texture(uTexSampler, vTexCoord);
}
