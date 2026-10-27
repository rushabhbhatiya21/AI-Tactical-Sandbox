// Jolt.h must be included before any other Jolt header
#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>

#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>
#include <algorithm>
#include <cmath>

#include "aiController.h"
#include <physicsWorld.h>
#include <physicsLayers.h>

AIController::AIController(GameObject& agent, GameObject& player, PhysicsWorld& world, JPH::CharacterVsCharacterCollisionSimple& collision)
	: aiAgent(agent), player(player), physicsWorld(world), characterCollision(collision)
{
	const float radius = 0.5f * std::max(aiAgent.transform.scale.x, aiAgent.transform.scale.z);
	halfHeight = 0.5f * aiAgent.transform.scale.y;
	const float cylinderHalfHeight = std::max(0.0f, halfHeight - radius);

	JPH::RefConst<JPH::Shape> shape = JPH::RotatedTranslatedShapeSettings(
		JPH::Vec3(0.0f, halfHeight, 0.0f),
		JPH::QuatArg::sIdentity(),
		new JPH::CapsuleShape(cylinderHalfHeight, radius)
	).Create().Get();

	JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
	settings->mShape = shape;
	settings->mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
	// Only contacts below the bottom sphere's centre count as standing on something
	settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -radius);

	const glm::vec3 feet = aiAgent.transform.position - glm::vec3(0.0f, halfHeight, 0.0f);
	character = new JPH::CharacterVirtual(
		settings, 
		JPH::RVec3(feet.x, feet.y, feet.z),
		JPH::Quat::sIdentity(), 
		0,
		&physicsWorld.getPhysicsSystem()
	);

	character->AddRef();
	character->SetCharacterVsCharacterCollision(&characterCollision);
}

AIController::~AIController()
{
	if (character)
	{
		characterCollision.Remove(character);
		character->Release();
	}
}

void AIController::update(float dt)
{
	JPH::PhysicsSystem& physicsSystem = physicsWorld.getPhysicsSystem();
	JPH::Vec3 gravity = physicsSystem.GetGravity();

	character->UpdateGroundVelocity();
	const JPH::Vec3 currentVerticalVelocity(0.0f, character->GetLinearVelocity().GetY(), 0.0f);
	const JPH::Vec3 groundVelocity = character->GetGroundVelocity();
	const bool onGround = character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround;

	JPH::Vec3 newVelocity;
	if (onGround && currentVerticalVelocity.GetY() - groundVelocity.GetY() < 0.1f)
	{
		newVelocity = groundVelocity;
	}
	else
	{
		newVelocity = currentVerticalVelocity;
	}

	newVelocity += gravity * dt;
	newVelocity += JPH::Vec3(movementDirection.x, 0.0f, movementDirection.z) * movementSpeed;
	character->SetLinearVelocity(newVelocity);

	JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
	character->ExtendedUpdate(
		dt,
		gravity,
		updateSettings,
		physicsSystem.GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		physicsSystem.GetDefaultLayerFilter(Layers::MOVING),
		JPH::BodyFilter(),
		JPH::ShapeFilter(),
		physicsWorld.getTempAllocator()
	);

	const JPH::Vec3 feet = character->GetPosition();
	aiAgent.transform.position = glm::vec3(
		static_cast<float>(feet.GetX()), 
		static_cast<float>(feet.GetY()) + halfHeight, 
		static_cast<float>(feet.GetZ())
	);
}

JPH::CharacterVirtual* AIController::getCharacter() const
{
	return character;
}

bool AIController::canSeePlayer() const
{
	if (!isPlayerInDetectionRange()) return false;
	glm::vec3 agentEyePosition = aiAgent.transform.position;
	agentEyePosition.y += 0.5f;
	glm::vec3 playerEyePosition = player.transform.position;
	playerEyePosition.y += 0.5f;
	return physicsWorld.hasLineOfSight(agentEyePosition, playerEyePosition);
}

AIDebugInfo AIController::getDebugInfo() const
{
	AIDebugInfo info;
	info.distanceToPlayer = glm::length(player.transform.position - aiAgent.transform.position);
	info.detectionRange = detectionRange;
	info.playerInRange = isPlayerInDetectionRange();
	info.canSeePlayer = canSeePlayer();
	return info;
}

bool AIController::isPlayerInDetectionRange() const
{
	glm::vec3 dist = player.transform.position - aiAgent.transform.position;
	return glm::dot(dist, dist) <= detectionRange * detectionRange;
}
