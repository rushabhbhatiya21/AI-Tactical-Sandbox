#pragma once
#include <glm/glm.hpp>

class Camera
{
public:
	Camera();
	~Camera();

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;
	void setAspectRatio(float ratio);

private:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	float fov = 45.0f;
	float aspectRatio = 16.0f / 9.0f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
};