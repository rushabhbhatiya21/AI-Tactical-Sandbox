#pragma once
#include <gameObject.h>

namespace JPH
{
	class CharacterVirtual;
}

class PhysicsWorld;

// Snapshot of what the agent currently knows, for the debug panel
struct AIDebugInfo
{
	bool canSeePlayer = false;
	bool playerInRange = false;
	float distanceToPlayer = 0.0f;
	float detectionRange = 0.0f;
};

class AIController
{
public:
	AIController(GameObject& aiAgent, GameObject& player, PhysicsWorld& world, JPH::CharacterVsCharacterCollisionSimple& characterCollision);
	~AIController();

	AIController(const AIController&) = delete;
	AIController& operator=(const AIController&) = delete;

	void update(float dt);
	JPH::CharacterVirtual* getCharacter() const;

	bool canSeePlayer() const;
	AIDebugInfo getDebugInfo() const;

private:
	bool isPlayerInDetectionRange() const;

private:
	GameObject& aiAgent;
	GameObject& player;
	PhysicsWorld& physicsWorld;
	JPH::CharacterVsCharacterCollisionSimple& characterCollision;

	JPH::CharacterVirtual* character = nullptr;

	float halfHeight = 0.0f;
	float movementSpeed = 2.0f;
	glm::vec3 movementDirection = glm::vec3(0.0f, 0.0f, 1.0f);

	float detectionRange = 25.0f;
};