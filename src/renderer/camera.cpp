#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
{
}

Camera::~Camera()
{
}

glm::mat4 Camera::getViewMatrix() const
{
	return glm::lookAt(position, target, up);
}

glm::mat4 Camera::getProjectionMatrix() const
{
	return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::setAspectRatio(float ratio)
{
	if (ratio > 0)
		aspectRatio = ratio;
}
