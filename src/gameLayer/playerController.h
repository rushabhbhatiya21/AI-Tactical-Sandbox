#pragma once
#include "gameObject.h"

namespace JPH
{
	class CharacterVirtual;
}

class InputState;
class PhysicsWorld;

class PlayerController
{
public:
	PlayerController(GameObject& player, PhysicsWorld& world, JPH::CharacterVsCharacterCollisionSimple& collision);
	~PlayerController();

	PlayerController(const PlayerController&) = delete;
	PlayerController& operator=(const PlayerController&) = delete;

	void update(float dt, const InputState& input, float yaw);
	JPH::CharacterVirtual* getCharacter() const;

private:
	GameObject& player;
	PhysicsWorld& physicsWorld;
	JPH::CharacterVsCharacterCollisionSimple& characterCollision;

	// Jolt ref-counts characters; we hold one reference (AddRef in the constructor, Release in the destructor)
	JPH::CharacterVirtual* character = nullptr;

	float movementSpeed = 5.0f;
	float jumpSpeed = 5.0f;    // initial upward speed; peak height = v^2 / 2g, about 1.3 m
	float halfHeight = 0.0f;
};
