#pragma once
#include "gameObject.h"

class InputState;

class PlayerController
{
public:
	PlayerController(GameObject& player);
	void update(float dt, const InputState& input);

private:
	float movementSpeed = 5.0f;
	GameObject& player;
};