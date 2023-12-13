#pragma once
#include "ECS.h"
#include "Components.h"
class AIComponent : public Component {
private:
	TransformComponent* AITransform, PlayerTransform;

public:

	AIComponent() {

	}

	void init() override {}

	void update() override {}
};