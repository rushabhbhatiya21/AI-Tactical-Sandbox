#include "playerController.h"
#include "input/inputState.h"

#include <glm/glm.hpp>

PlayerController::PlayerController(GameObject& player)
	: player(player)
{
}

void PlayerController::update(float dt, const InputState& input)
{
	glm::vec3 movementVector = glm::vec3(0.0f);

	if (input.moveForward)
		movementVector.z -= 1.0f;
	if (input.moveBackward)
		movementVector.z += 1.0f;
	if (input.moveLeft)
		movementVector.x -= 1.0f;
	if (input.moveRight)
		movementVector.x += 1.0f;

	if (glm::dot(movementVector, movementVector) < 1e-6f)
		return;

	movementVector = glm::normalize(movementVector);
	movementVector *= movementSpeed * dt;
	player.transform.position += movementVector;
}
