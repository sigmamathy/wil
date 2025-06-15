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

struct LightData {
	vec3 pos;
	vec3 color;
};

layout(std140, set = 0, binding = 1) readonly buffer Lights {
	LightData data[100];
	uint count;
} uLights;

layout(set = 1, binding = 0) uniform sampler2D uTexSampler;

const float ambient_strength = 0.02f;
const float specular_strength = 0.5f;

vec3 calculate_light(LightData data)
{
	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(data.pos - vFragPos);  

	vec3 viewDir = normalize(uGlobal.viewPos - vFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	vec3 ambient = ambient_strength * data.color;
	vec3 diffuse = max(dot(norm, lightDir), 0.0) * data.color;

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specular_strength * spec * data.color;  

	return ambient + diffuse + specular;
}

void main()
{
	vec3 light_color = vec3(0.f,0.f,0.f);

	for (uint i = 0; i < uLights.count; ++i)
		light_color += calculate_light(uLights.data[i]);

	oFragColor = vec4(light_color, 1.f) * texture(uTexSampler, vTexCoord);
}
