#pragma once
#include "ECS.h"
#include <string>
#include "Components.h"
#include "TextureManager.h"

class ColliderComponent : public Component {
public:
	SDL_Rect collider;
	std::string tag;

	bool visible = true;

	int typeofWall;


	SDL_Texture* tex;
	SDL_Rect srcRect, dstRect;

	TransformComponent* transform;

	ColliderComponent(std::string s) {
		tag = s;
	}

	ColliderComponent(std::string t, int xpos, int ypos, int size) {
		visible = false;
		tag = t;
		collider.x = xpos;
		collider.y = ypos;
		collider.h = collider.w = size;
	}

	ColliderComponent(std::string t, int xpos, int ypos, int p_w, int p_h, int type) {
		tag = t;
		collider.x = xpos;
		collider.y = ypos;
		collider.h = p_h;
		collider.w = p_w;

		typeofWall = type;
	}

	void init() override {
		if (!entity->hasComponent<TransformComponent>()) {
			entity->addComponent<TransformComponent>();
		}
		transform = &entity->getComponent<TransformComponent>();

		if (visible) {
			switch (typeofWall) {
			case 1:
				tex = TextureManager::LoadTexture("assets/ColTex1.png");
				break;
			case 2:
				tex = TextureManager::LoadTexture("assets/ColTex2.png");
				break;
			case 3:
				tex = TextureManager::LoadTexture("assets/ColTex3.png");
				break;
			case 4:
				tex = TextureManager::LoadTexture("assets/ColTex4.png");
				break;
			}
		}

		srcRect = { 0,0,32,32 };
		dstRect = { collider.x, collider.y, collider.w, collider.h };

	}

	void update() override {
		if (tag != "terrain" && tag != "elev" && tag != "ladd") {
			collider.x = static_cast<int>(transform->position.x);
			collider.y = static_cast<int>(transform->position.y);
			collider.w = static_cast<int>(transform->width * transform->scale);
			collider.h = static_cast<int>(transform->height * transform->scale);
		}
		dstRect.x = collider.x - Game::camera.x;
		dstRect.y = collider.y - Game::camera.y;
	}

	void draw() override {
		if (visible)
			TextureManager::Draw(tex, srcRect, dstRect, SDL_FLIP_NONE);
	}
};