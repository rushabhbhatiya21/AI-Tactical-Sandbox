#version 460 core
out vec4 FragColor;

struct Material
{
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Light {
    vec3 position;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform vec3 viewPos;
uniform Material material;
uniform Light light;

in vec3 Normal;
in vec3 FragPos;


void main()
{
	// ambient lighting
	vec3 ambient = light.ambient * material.diffuse;

	// diffuse lighting
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(light.position - FragPos);
	float diff = max(dot(norm, lightDir), 0.0f);
	vec3 diffuse = light.diffuse * diff * material.diffuse;

	// specular lighting
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
	vec3 specular = light.specular * spec * material.specular;

	// final lighting
	FragColor = vec4(ambient + diffuse + specular, 1.0f);
}