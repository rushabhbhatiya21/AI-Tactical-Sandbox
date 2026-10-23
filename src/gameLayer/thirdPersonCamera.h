#pragma once
#include <glad/glad.h> // include glad to get all the required OpenGL headers
#include <glm/glm.hpp>

#include "gameObject.h"
#include "ICamera.h"

class ThirdPersonCamera : public ICamera
{
public:
	ThirdPersonCamera(GameObject& player);
	void update(float dt);
	void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	float getYaw() const;

	// interface methods
	glm::mat4 getViewMatrix() const override;
	glm::mat4 getProjectionMatrix() const override;
	glm::vec3 getPosition() const override;

private:
	GameObject& player;
	glm::vec3 position;
	glm::vec3 offset = glm::vec3(0.0f, 3.0f, 6.0f);
	glm::mat4 viewMatrix;

	float yaw = -90.0f;
	float pitch = 0.0f;
	float mouseSensitivity = 0.1f;
	float distance = 6.0f;
	float heightOffset = 2.0f;

	float fov = 45.0f;
	float aspectRatio = 16.0f / 9.0f;
	float nearPlane = 0.1f;
	float farPlane = 200.0f;
};