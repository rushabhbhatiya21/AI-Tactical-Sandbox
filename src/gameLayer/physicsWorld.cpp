// Jolt.h must be included before any other Jolt header
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollector.h>

#include "physicsWorld.h"
#include "physicsLayers.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <thread>

#pragma region layer setup

// Maps each object layer to a broad-phase layer (one-to-one here).
class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	JPH::uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
	{
		JPH_ASSERT(layer < Layers::NUM_LAYERS);
		return layer == Layers::NON_MOVING ? BroadPhaseLayers::NON_MOVING : BroadPhaseLayers::MOVING;
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
	{
		return layer == BroadPhaseLayers::NON_MOVING ? "NON_MOVING" : "MOVING";
	}
#endif
};

// Can an object in this layer collide with anything in that broad-phase layer?
class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broadPhaseLayer) const override
	{
		switch (layer)
		{
		case Layers::NON_MOVING: return broadPhaseLayer == BroadPhaseLayers::MOVING;
		case Layers::MOVING:     return true;
		default:                 return false;
		}
	}
};

// Can these two object layers collide? Static vs static never needs testing.
class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
	{
		switch (a)
		{
		case Layers::NON_MOVING: return b == Layers::MOVING;
		case Layers::MOVING:     return true;
		default:                 return false;
		}
	}
};

// LOS query considers static arena geometry only, ignoring moving objects
class StaticOnlyObjectLayerFilter final : public JPH::ObjectLayerFilter
{
public:
	bool ShouldCollide(JPH::ObjectLayer layer) const override
	{
		return layer == Layers::NON_MOVING;
	}
};

#pragma endregion

#pragma region jolt callbacks

static void traceImpl(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	std::cout << "[JOLT] " << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS
static bool assertFailedImpl(const char* expression, const char* message, const char* file, JPH::uint line)
{
	std::cout << "[JOLT ASSERT] " << file << ":" << line << ": (" << expression << ") "
		<< (message ? message : "") << std::endl;
	return true; // break into the debugger
}
#endif

#pragma endregion

PhysicsWorld::PhysicsWorld() = default;

PhysicsWorld::~PhysicsWorld()
{
	shutdown();
}

void PhysicsWorld::init()
{
	if (initialized) return;

	JPH::RegisterDefaultAllocator();
	JPH::Trace = traceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = assertFailedImpl;)

	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterTypes();

	tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

	// Leave one core for the main thread
	int workerThreads = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
	jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
		JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, workerThreads);

	broadPhaseLayerInterface = std::make_unique<BroadPhaseLayerInterfaceImpl>();
	objectVsBroadPhaseLayerFilter = std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();
	objectLayerPairFilter = std::make_unique<ObjectLayerPairFilterImpl>();

	const JPH::uint maxBodies = 1024;
	const JPH::uint numBodyMutexes = 0; // 0 = Jolt picks a default
	const JPH::uint maxBodyPairs = 1024;
	const JPH::uint maxContactConstraints = 1024;

	physicsSystem = std::make_unique<JPH::PhysicsSystem>();
	physicsSystem->Init(maxBodies, numBodyMutexes, maxBodyPairs, maxContactConstraints,
		*broadPhaseLayerInterface, *objectVsBroadPhaseLayerFilter, *objectLayerPairFilter);
	physicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

	initialized = true;
}

void PhysicsWorld::update(float dt)
{
	if (!initialized) return;

	// Step at a fixed rate regardless of frame time
	accumulator += dt;
	int steps = 0;
	while (accumulator >= fixedTimeStep && steps < maxStepsPerFrame)
	{
		physicsSystem->Update(fixedTimeStep, 1, tempAllocator.get(), jobSystem.get());
		accumulator -= fixedTimeStep;
		++steps;
	}

	// Drop leftover time if we hit the cap, so we don't fall further behind
	if (steps == maxStepsPerFrame)
		accumulator = 0.0f;
}

void PhysicsWorld::shutdown()
{
	if (!initialized) return;

	JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();
	for (uint32_t id : bodyIds)
	{
		JPH::BodyID bodyId(id);
		bodyInterface.RemoveBody(bodyId);
		bodyInterface.DestroyBody(bodyId);
	}
	bodyIds.clear();

	// Destroy in reverse order of creation; the system references the layer objects
	physicsSystem.reset();
	objectLayerPairFilter.reset();
	objectVsBroadPhaseLayerFilter.reset();
	broadPhaseLayerInterface.reset();
	jobSystem.reset();
	tempAllocator.reset();

	JPH::UnregisterTypes();
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;

	initialized = false;
}

void PhysicsWorld::addStaticBox(const glm::vec3& center, const glm::vec3& halfExtents, const glm::vec3& rotationDegrees)
{
	// Transform::getModelMatrix() builds Rx * Ry * Rz, so compose the quaternion in the same order,
	// otherwise the collider won't line up with what's drawn.
	const JPH::Quat rotation =
		JPH::Quat::sRotation(JPH::Vec3::sAxisX(), JPH::DegreesToRadians(rotationDegrees.x)) *
		JPH::Quat::sRotation(JPH::Vec3::sAxisY(), JPH::DegreesToRadians(rotationDegrees.y)) *
		JPH::Quat::sRotation(JPH::Vec3::sAxisZ(), JPH::DegreesToRadians(rotationDegrees.z));

	JPH::BodyCreationSettings settings(
		new JPH::BoxShape(JPH::Vec3(halfExtents.x, halfExtents.y, halfExtents.z)),
		JPH::RVec3(center.x, center.y, center.z),
		rotation.Normalized(),
		JPH::EMotionType::Static,
		Layers::NON_MOVING);

	JPH::BodyID id = physicsSystem->GetBodyInterface().CreateAndAddBody(settings, JPH::EActivation::DontActivate);
	bodyIds.push_back(id.GetIndexAndSequenceNumber());
}

bool PhysicsWorld::hasLineOfSight(const glm::vec3 origin, const glm::vec3 target)
{
	if (!initialized)
		return false;

	const glm::vec3 direction = target - origin;

	// A zero-length ray has no obstruction to test.
	if (glm::dot(direction, direction) < 1e-6f)
		return true;

	const JPH::RRayCast ray(
		JPH::RVec3(origin.x, origin.y, origin.z),
		JPH::Vec3(direction.x, direction.y, direction.z)
	);

	JPH::RayCastResult hit;

	static const StaticOnlyObjectLayerFilter staticFilter;

	const bool blocked = physicsSystem->GetNarrowPhaseQuery().CastRay(
		ray, 
		hit, 
		physicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING),
		staticFilter
	);

	return !blocked;
}

JPH::PhysicsSystem& PhysicsWorld::getPhysicsSystem()
{
	return *physicsSystem;
}

JPH::TempAllocatorImpl& PhysicsWorld::getTempAllocator()
{
	return *tempAllocator;
}
