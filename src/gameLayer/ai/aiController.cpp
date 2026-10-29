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
#include <sound/soundEvent.h>

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
	heardSoundThisFrame = false;

	updateVisualPerception(dt);
	updateSoundPerception(dt);

	// Timers
	if (searchTimer      > 0.0f) searchTimer -= dt;
	if (investigateTimer > 0.0f) investigateTimer -= dt;

	updateDecision();
	updateMovementDirection();

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
	if (!isPlayerWithinFOV()) return false;
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
	info.detectionRange = DETECTION_RANGE;
	info.playerInRange = isPlayerInDetectionRange();
	info.playerWithinFov = isPlayerWithinFOV();
	info.canSeePlayer = canSeePlayer();
	info.lastKnownLocation = lastKnownLocation;
	info.timeSinceLastSeenPlayer = timeSinceLastSeenPlayer;
	info.hasRecentMemory = hasRecentMemory();
	info.visualPerceptionState = getVisualPerceptionState();
	info.lastKnownSoundLocation = lastKnownSoundLocation;
	info.hasHeardSound = hasHeardSound;
	info.hasRecentSoundMemory = hasRecentSoundMemory();
	info.auditoryPerceptionState = getAuditoryPerceptionState();

	return info;
}

VisualPerceptionState AIController::getVisualPerceptionState() const
{
	if (canSeePlayer()) return VisualPerceptionState::Visible;
	else if (hasRecentMemory()) return VisualPerceptionState::Remembered;
	else return VisualPerceptionState::Unknown;
}

AuditoryPerceptionState AIController::getAuditoryPerceptionState() const
{
	if (heardSoundThisFrame) return AuditoryPerceptionState::Heard;
	if (hasRecentSoundMemory()) return AuditoryPerceptionState::Remembered;
	else return AuditoryPerceptionState::Unknown;
}

void AIController::processSoundEvent(const SoundEvent& sound)
{
	if (canHearSound(sound))
	{
		lastKnownSoundLocation = sound.origin;
		hasHeardSound = true;
		heardSoundThisFrame = true;
		timeSinceLastHeardSound = 0.0f;
	}
}

bool AIController::isPlayerInDetectionRange() const
{
	glm::vec3 dist = player.transform.position - aiAgent.transform.position;
	return glm::dot(dist, dist) <= DETECTION_RANGE * DETECTION_RANGE;
}

bool AIController::isPlayerWithinFOV() const
{
	glm::vec3 dir = player.transform.position - aiAgent.transform.position;
	if (glm::dot(dir, dir) >= 0.0001f)
		dir = glm::normalize(dir);
	glm::vec3 forward = glm::vec3(sin(glm::radians(aiAgent.transform.rotation.y)), 0.0f, cos(glm::radians(aiAgent.transform.rotation.y)));
	if (glm::dot(forward, dir) >= glm::cos(glm::radians(45.0f)))
		return true;
	return false;
}

void AIController::updateVisualPerception(float dt)
{
	if (canSeePlayer())
	{
		lastKnownLocation = player.transform.position;
		hasAcquiredValidPosition = true;
		timeSinceLastSeenPlayer = 0.0f;
	}
	else if (hasAcquiredValidPosition)
	{
		timeSinceLastSeenPlayer += dt;
	}
}

bool AIController::hasRecentMemory() const
{
	return hasAcquiredValidPosition && timeSinceLastSeenPlayer < memoryDuration;
}

bool AIController::canHearSound(const SoundEvent& sound) const
{
	glm::vec3 difference = aiAgent.transform.position - sound.origin;
	float distanceSquared = glm::dot(difference, difference);
	float radiusSquared = sound.radius * sound.radius;

	return distanceSquared <= radiusSquared;
}

void AIController::updateSoundPerception(float dt)
{
	if (!hasHeardSound)
		return;

	timeSinceLastHeardSound += dt;

	if (timeSinceLastHeardSound >= soundMemoryDuration)
		hasHeardSound = false;
}

bool AIController::hasRecentSoundMemory() const
{
	return hasHeardSound && timeSinceLastHeardSound < soundMemoryDuration;
}

void AIController::updateDecision()
{
	const bool playerVisible = canSeePlayer();

	if (playerVisible)
	{
		enterState(AIState::Engage);
		return;
	}

	if (currentState == AIState::Engage && !playerVisible)
	{
		if (hasRecentMemory())
			enterState(AIState::Search);
		else if (hasRecentSoundMemory())
			enterState(AIState::Investigate);
		else
			enterState(AIState::Unaware);
		return;
	}

	if (currentState == AIState::Search && searchTimer <= 0.0f)
	{
		if (hasRecentSoundMemory())
		{
			enterState(AIState::Investigate);
			return;
		}
		enterState(AIState::Unaware);
		return;
	}

	if (currentState == AIState::Investigate && investigateTimer <= 0.0f)
	{
		if (hasRecentMemory())
		{
			enterState(AIState::Search);
			return;
		}
		enterState(AIState::Unaware);
		return;
	}

	if (currentState == AIState::Unaware)
	{
		// Unaware branch should respond to recent sound memory by entering Investigate, 
		// but it should not automatically enter Search just because visual memory exists.
		if (hasRecentSoundMemory())
		{
			enterState(AIState::Investigate);
			return;
		}
	}
}

void AIController::updateMovementDirection()
{
	switch (currentState)
	{
	case AIState::Unaware:
		// direction should be towards patrol waypoints later
		movementDirection = glm::vec3(0.0f);
		break;
	case AIState::Engage:
	{
		glm::vec3 dir = player.transform.position - aiAgent.transform.position;
		dir.y = 0.0f; // ignore vertical dir
		if (glm::dot(dir, dir) > 1e-6f)
			dir = glm::normalize(dir);
		movementDirection = dir;
		break;
	}
	case AIState::Investigate:
	{
		glm::vec3 dir = lastKnownSoundLocation - aiAgent.transform.position;
		dir.y = 0.0f; // ignore vertical dir
		if (glm::dot(dir, dir) <= ARRIVAL_DISTANCE * ARRIVAL_DISTANCE)
			dir = glm::vec3();
		else
			dir = glm::normalize(dir);
		movementDirection = dir;
		break;
	}
	case AIState::Search:
	{
		glm::vec3 dir = lastKnownLocation - aiAgent.transform.position;
		dir.y = 0.0f; // ignore vertical dir
		if (glm::dot(dir, dir) <= ARRIVAL_DISTANCE * ARRIVAL_DISTANCE)
			dir = glm::vec3();
		else
			dir = glm::normalize(dir);
		movementDirection = dir;
		break;
	}
	default:
		break;
	}

	if (glm::dot(movementDirection, movementDirection) < 1e-6f)
	{
		float yawRadians = std::atan2(movementDirection.x, movementDirection.z);
		aiAgent.transform.rotation.y = glm::degrees(yawRadians);
	}
}

void AIController::enterState(AIState newState)
{
	if (currentState == newState)
		return;

	previousState = currentState;
	currentState = newState;

	switch (currentState)
	{
	case AIState::Unaware:
		break;
	case AIState::Engage:
		break;
	case AIState::Investigate:
		investigateTimer = INVESTIGATE_TIME;
		break;
	case AIState::Search:
		searchTimer = SEARCH_TIME;
		break;
	default:
		break;
	}
}
