#pragma once
#include "gameObject.h"

class InputState;

class PlayerController
{
public:
	PlayerController(GameObject& player);
	void update(float dt, const InputState& input, float yaw);

private:
	GameObject& player;
	float movementSpeed = 5.0f;
};