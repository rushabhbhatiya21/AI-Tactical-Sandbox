#pragma once
#include <gameObject.h>

namespace JPH
{
	class CharacterVirtual;
}

class PhysicsWorld;

class AIController
{
public:
	AIController(GameObject& aiAgent, PhysicsWorld& world);
	~AIController();

	AIController(const AIController&) = delete;
	AIController& operator=(const AIController&) = delete;

	void update(float dt);

private:
	GameObject& aiAgent;
	PhysicsWorld& physicsWorld;

	JPH::CharacterVirtual* character = nullptr;

	float halfHeight = 0.0f;
	float movementSpeed = 2.0f;
	glm::vec3 movementDirection = glm::vec3(0.0f, 0.0f, 1.0f);
};