#pragma once
#include "types/transform.h"
#include "renderComponent.h"
#include <memory>

class GameObject
{
public:
	Transform transform;
	std::unique_ptr<RenderComponent> renderComponent;
};