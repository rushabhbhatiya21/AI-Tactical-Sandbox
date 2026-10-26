#include "physicsWorldTests.h"
#include <iostream>

void runLineOfSightTests(PhysicsWorld& physicsWorld)
{
	const bool clearPath = physicsWorld.hasLineOfSight(
		glm::vec3(-12.0f, 1.5f, -25.0f),
		glm::vec3(12.0f, 1.5f, -25.0f));

	std::cout << "Clear path: " << clearPath << '\n';

	const bool blockedPath = physicsWorld.hasLineOfSight(
		glm::vec3(23.0f, 1.5f, 5.0f),
		glm::vec3(23.0f, 1.5f, 21.0f));

	std::cout << "Blocked path: " << blockedPath << '\n';
}
