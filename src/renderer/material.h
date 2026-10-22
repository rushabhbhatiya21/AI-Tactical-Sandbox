#pragma once
#include <glm/glm.hpp>

class Material
{
public:
	//glm::vec4 color = glm::vec4(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(0.5f);
	float shininess = 32.0f;
};