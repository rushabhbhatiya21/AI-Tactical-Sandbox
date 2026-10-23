#include "thirdPersonCamera.h"
#include <glm/gtc/matrix_transform.hpp>

ThirdPersonCamera::ThirdPersonCamera(GameObject& player)
	: player(player)
{
}

void ThirdPersonCamera::update(float dt)
{
    // calculate the new orbitDir vector
    glm::vec3 orbitDir;
    orbitDir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    orbitDir.y = sin(glm::radians(pitch));
    orbitDir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    orbitDir = glm::normalize(orbitDir);

    const glm::vec3 target = player.transform.position + glm::vec3(0.0f, heightOffset, 0.0f);

    // Place the camera on a sphere of radius `distance` around the target.
    position = target - orbitDir * distance;

	viewMatrix = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void ThirdPersonCamera::processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch)
    {
        if (pitch > 60.0f)
            pitch = 60.0f;
        if (pitch < -30.0f)
            pitch = -30.0f;
    }
}

float ThirdPersonCamera::getYaw() const
{
    return yaw;
}

glm::mat4 ThirdPersonCamera::getViewMatrix() const
{
	return viewMatrix;
}

glm::mat4 ThirdPersonCamera::getProjectionMatrix() const
{
	return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

glm::vec3 ThirdPersonCamera::getPosition() const
{
	return position;
}
