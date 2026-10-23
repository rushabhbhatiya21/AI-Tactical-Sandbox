#pragma once
#include <glm/glm.hpp>

#include <cstdint>
#include <memory>
#include <vector>

// Forward declarations keep Jolt headers out of everything that includes this file.
namespace JPH
{
	class PhysicsSystem;
	class TempAllocatorImpl;
	class JobSystemThreadPool;
}

class BroadPhaseLayerInterfaceImpl;
class ObjectVsBroadPhaseLayerFilterImpl;
class ObjectLayerPairFilterImpl;

class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	void init();
	void update(float dt);
	void shutdown();

	// Adds an immovable box collider. halfExtents is half the box size on each axis.
	// rotationDegrees uses the same Euler convention as Transform (X, then Y, then Z in the model matrix).
	void addStaticBox(const glm::vec3& center, const glm::vec3& halfExtents,
		const glm::vec3& rotationDegrees = glm::vec3(0.0f));

	JPH::PhysicsSystem& getPhysicsSystem();
	JPH::TempAllocatorImpl& getTempAllocator();

private:
	std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator;
	std::unique_ptr<JPH::JobSystemThreadPool> jobSystem;
	std::unique_ptr<BroadPhaseLayerInterfaceImpl> broadPhaseLayerInterface;
	std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl> objectVsBroadPhaseLayerFilter;
	std::unique_ptr<ObjectLayerPairFilterImpl> objectLayerPairFilter;
	std::unique_ptr<JPH::PhysicsSystem> physicsSystem;

	// Stored as Jolt's packed index + sequence number so JPH::BodyID isn't needed here
	std::vector<uint32_t> bodyIds;

	float accumulator = 0.0f;
	static constexpr float fixedTimeStep = 1.0f / 60.0f;
	static constexpr int maxStepsPerFrame = 4;   // avoids a spiral of death after a long hitch
	bool initialized = false;
};
