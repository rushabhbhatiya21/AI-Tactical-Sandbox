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

#include "playerController.h"
#include "physicsWorld.h"
#include "physicsLayers.h"
#include "input/inputState.h"

PlayerController::PlayerController(GameObject& player, PhysicsWorld& world, JPH::CharacterVsCharacterCollisionSimple& collision)
	: player(player), physicsWorld(world), characterCollision(collision)
{
	// Capsule that fits the player's box: radius from the footprint, total height from scale.y
	const float radius = 0.5f * std::max(player.transform.scale.x, player.transform.scale.z);
	halfHeight = 0.5f * player.transform.scale.y;
	const float cylinderHalfHeight = std::max(0.0f, halfHeight - radius);

	// Shift the capsule up so the character's position is at its feet
	JPH::RefConst<JPH::Shape> shape = JPH::RotatedTranslatedShapeSettings(
		JPH::Vec3(0.0f, halfHeight, 0.0f),
		JPH::Quat::sIdentity(),
		new JPH::CapsuleShape(cylinderHalfHeight, radius)).Create().Get();

	JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
	settings->mShape = shape;
	settings->mMaxSlopeAngle = JPH::DegreesToRadians(45.0f);
	// Only contacts below the bottom sphere's centre count as standing on something
	settings->mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -radius);

	const glm::vec3 feet = player.transform.position - glm::vec3(0.0f, halfHeight, 0.0f);
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

PlayerController::~PlayerController()
{
	if (character)
	{
		characterCollision.Remove(character);
		character->Release();
	}
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

	// 3. Camera's horizontal basis, matching ThirdPersonCamera::update().
	//    Camera looks along (cos yaw, 0, sin yaw); its right is (-sin yaw, 0, cos yaw).
	float yawRad = glm::radians(cameraYawDegrees);
	glm::vec3 camForward(glm::cos(yawRad), 0.0f, glm::sin(yawRad));
	glm::vec3 camRight(-glm::sin(yawRad), 0.0f, glm::cos(yawRad));

	// 4. Combine into a world-space direction. Normalize so diagonals aren't faster.
	glm::vec3 worldMovement = camForward * forwardInput + camRight * rightInput;
	if (forwardInput != 0.0f || rightInput != 0.0f)
		worldMovement = glm::normalize(worldMovement);

	// 5. Build the character's velocity: keep falling speed while airborne, reset it once grounded.
	JPH::PhysicsSystem& physicsSystem = physicsWorld.getPhysicsSystem();
	const JPH::Vec3 gravity = physicsSystem.GetGravity();

	character->UpdateGroundVelocity();
	const JPH::Vec3 currentVerticalVelocity(0.0f, character->GetLinearVelocity().GetY(), 0.0f);
	const JPH::Vec3 groundVelocity = character->GetGroundVelocity();
	const bool onGround = character->GetGroundState() == JPH::CharacterVirtual::EGroundState::OnGround;

	JPH::Vec3 newVelocity;
	if (onGround && currentVerticalVelocity.GetY() - groundVelocity.GetY() < 0.1f)
		newVelocity = groundVelocity;
	else
		newVelocity = currentVerticalVelocity;

	// Jump only from the ground, so holding or mashing space in the air does nothing
	if (onGround && input.jump)
		newVelocity += JPH::Vec3(0.0f, jumpSpeed, 0.0f);

	newVelocity += gravity * dt;
	newVelocity += JPH::Vec3(worldMovement.x, 0.0f, worldMovement.z) * movementSpeed;
	character->SetLinearVelocity(newVelocity);

	// 6. Move the character; Jolt slides it along walls, handles steps and keeps it on the floor.
	JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
	character->ExtendedUpdate(dt, gravity, updateSettings,
		physicsSystem.GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		physicsSystem.GetDefaultLayerFilter(Layers::MOVING),
		JPH::BodyFilter(), JPH::ShapeFilter(),
		physicsWorld.getTempAllocator());

	// 7. Copy the result back for rendering (the transform is the centre, the character is at the feet).
	const JPH::RVec3 feet = character->GetPosition();
	player.transform.position = glm::vec3(
		static_cast<float>(feet.GetX()),
		static_cast<float>(feet.GetY()) + halfHeight,
		static_cast<float>(feet.GetZ()));
}

JPH::CharacterVirtual* PlayerController::getCharacter() const
{
	return character;
}
