#pragma once
#include "gameObject.h"
#include <vector>
#include <memory>

class World
{
public:
	GameObject& createGameObject();
	const std::vector<std::unique_ptr<GameObject>>& getGameObjects() const;

	void update(float dt);
	void clear();
private:
	std::vector<std::unique_ptr<GameObject>> gameObjects;
};