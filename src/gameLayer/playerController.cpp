#include "playerController.h"
#include "input/inputState.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <cmath>

PlayerController::PlayerController(GameObject& player)
	: player(player)
{
}

void PlayerController::update(float dt, const InputState& input, float cameraYawDegrees)
{
	// 1. Face where the camera (mouse) is aiming.
	//    Camera looks along (cos yaw, 0, sin yaw). For a model that faces -Z at rotation.y = 0,
	//    glm::rotate about +Y maps local -Z to (-sin θ, 0, -cos θ), so θ = atan2(-x, -z) = -yaw - 90.
	//    Camera yaw is unbounded, so wrap into [0, 360).
	float facingYaw = std::fmod(-cameraYawDegrees - 90.0f, 360.0f);
	player.transform.rotation.y = facingYaw < 0.0f ? facingYaw + 360.0f : facingYaw;

	// 2. Read raw input in camera space.
	//    +forwardInput = away from the camera, +rightInput = camera's right.
	float forwardInput = 0.0f;
	float rightInput = 0.0f;

	if (input.moveForward)  forwardInput += 1.0f;
	if (input.moveBackward) forwardInput -= 1.0f;
	if (input.moveRight)    rightInput += 1.0f;
	if (input.moveLeft)     rightInput -= 1.0f;

	// No movement input: keep the current position.
	if (forwardInput == 0.0f && rightInput == 0.0f)
		return;

	// 3. Camera's horizontal basis, matching ThirdPersonCamera::update().
	//    Camera looks along (cos yaw, 0, sin yaw); its right is (-sin yaw, 0, cos yaw).
	float yawRad = glm::radians(cameraYawDegrees);
	glm::vec3 camForward(glm::cos(yawRad), 0.0f, glm::sin(yawRad));
	glm::vec3 camRight(-glm::sin(yawRad), 0.0f, glm::cos(yawRad));

	// 4. Combine into a world-space direction. Normalize so diagonals aren't faster.
	glm::vec3 worldMovement = camForward * forwardInput + camRight * rightInput;
	worldMovement = glm::normalize(worldMovement);

	// 5. Move the player (strafing; facing is driven by the mouse, not movement).
	player.transform.position += worldMovement * movementSpeed * dt;
}
