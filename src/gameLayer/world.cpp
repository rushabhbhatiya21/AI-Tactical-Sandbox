#include "world.h"

GameObject& World::createGameObject()
{
	std::unique_ptr<GameObject> gameObject = std::make_unique<GameObject>();
	GameObject& ref = *gameObject;
	gameObjects.push_back(std::move(gameObject));
	return ref;
}

const std::vector<std::unique_ptr<GameObject>>& World::getGameObjects() const
{
	return gameObjects;
}

void World::update(float dt)
{
	// Object update behavior will be added when needed.
}

void World::clear()
{
	gameObjects.clear();
}
