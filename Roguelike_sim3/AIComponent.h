#pragma once
#include "Game.h"
#include "ECS.h"
#include "Components.h"
#include <cmath>

#define RIGHT 1
#define LEFT 2
#define UP 3
#define DOWN 4

class AIComponent : public Component {
public:
	TransformComponent* transform;
	SpriteComponent* sprite;
	std::vector<int> curpath;

	bool active = true;

	int direction = 1;
	int cur = 0, next = 1;
	int curx = 0, cury = 0;

	void init() override {
		transform = &entity->getComponent<TransformComponent>();
		sprite = &entity->getComponent<SpriteComponent>();
	}

	void update() override {
		if (active) {
			if (std::abs(curx - transform->position.x) + std::abs(cury - transform->position.y) >= 128) {
				transform->velocity.x = 0;
				transform->velocity.y = 0;
				cur = next;
				next++;
				curx = transform->position.x;
				cury = transform->position.y;
				if (curpath[next] - curpath[cur] == -1) {
					direction = LEFT;
					transform->velocity.x = -1;
				}
				else if (curpath[next] - curpath[cur] == 1) {
					direction = RIGHT;
					transform->velocity.x = 1;
				}
				else if (curpath[next] - curpath[cur] < -1) {
					direction = UP;
					transform->velocity.y = -1;
				}
				else if (curpath[next] - curpath[cur] > 1) {
					direction = DOWN;
					transform->velocity.y = 1;
				}
			}

			switch (direction) {
			case LEFT:
				sprite->spriteflip = SDL_FLIP_HORIZONTAL;
				sprite->Play("Walk");
				break;
			case RIGHT:
				sprite->spriteflip = SDL_FLIP_NONE;
				sprite->Play("Walk");
				break;
			case UP:
				sprite->Play("Walk");
				break;
			case DOWN:
				sprite->Play("Walk");
				break;
			}
		}
	}

	void init_path(std::vector<int> temp_path) {
		cur = 0;
		next = 1;
		curpath = temp_path;
		active = true;
		if (curpath[next] - curpath[cur] == -1) {
			direction = LEFT;
			transform->velocity.x = -1;
		}
		else if (curpath[next] - curpath[cur] == 1) {
			direction = RIGHT;
			transform->velocity.x = 1;
		}
		else if (curpath[next] - curpath[cur] < -1) {
			direction = UP;
			transform->velocity.y = -1;
		}
		else if (curpath[next] - curpath[cur] > 1) {
			direction = DOWN;
			transform->velocity.y = 1;
		}
		curx = transform->position.x;
		cury = transform->position.y;
	}

	void killed() {
		active = false;
		transform->position.x = 7 * 128 + 50;
		transform->position.y = 7 * 128 + 50;
		transform->velocity.x = 0;
		transform->velocity.y = 0;
	}
};