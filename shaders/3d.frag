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

struct DirectionalLight {
	vec3 dir;
	vec3 color;
};

struct PointLight {
	vec3 pos;
	vec3 color;
	float linear;
	float quadratic;
};

struct SpotLight {
	vec3 pos;
	vec3 dir;
	vec3 color;
	float cutoff;
	float linear;
	float quadratic;
};

layout(std140, set = 0, binding = 1) readonly buffer Lights {
	DirectionalLight dl;
	PointLight pl[100];
	SpotLight sl[100];
	uint pl_count;
	uint sl_count;
} uLights;

layout(set = 1, binding = 0) uniform sampler2D uTexSampler;

const float ambient_strength = 0.02f;
const float specular_strength = 0.5f;

vec3 calculate_dir_light(DirectionalLight light)
{
	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(light.dir);  

	vec3 viewDir = normalize(uGlobal.viewPos - vFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	vec3 ambient = ambient_strength * light.color;
	vec3 diffuse = max(dot(norm, lightDir), 0.0) * light.color;

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specular_strength * spec * light.color;

	return ambient + diffuse + specular;
}

vec3 calculate_point_lights(PointLight light)
{
	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(light.pos - vFragPos);  

	vec3 viewDir = normalize(uGlobal.viewPos - vFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	vec3 ambient = ambient_strength * light.color;
	vec3 diffuse = max(dot(norm, lightDir), 0.0) * light.color;

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specular_strength * spec * light.color;

	float dist = length(light.pos - vFragPos);
	float attenuation = 1.0 / (1.0 + light.linear * dist + 
			light.quadratic * (dist * dist));  

	return attenuation * (ambient + diffuse + specular);
}

vec3 calculate_spot_lights(SpotLight light)
{
	vec3 norm = normalize(vNormal);
	vec3 lightDir = normalize(light.pos - vFragPos);

	float theta = dot(lightDir, normalize(-light.dir));

	vec3 ambient = ambient_strength * light.color;

	float dist = length(light.pos - vFragPos);
	float attenuation = 1.0 / (1.0 + light.linear * dist + 
			light.quadratic * (dist * dist));

	if (theta > light.cutoff) {
		return attenuation * ambient;
	}

	vec3 diffuse = max(dot(norm, lightDir), 0.0) * light.color;

	vec3 viewDir = normalize(uGlobal.viewPos - vFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specular_strength * spec * light.color;

	return attenuation * (ambient + diffuse + specular);
}

void main()
{
	vec3 light = vec3(0.f);

	light += calculate_dir_light(uLights.dl);

	for (uint i = 0; i < uLights.pl_count; ++i)
		light += calculate_point_lights(uLights.pl[i]);

	for (uint i = 0; i < uLights.sl_count; ++i)
		light += calculate_spot_lights(uLights.sl[i]);

	oFragColor = vec4(light, 1.f) * texture(uTexSampler, vTexCoord);
}
