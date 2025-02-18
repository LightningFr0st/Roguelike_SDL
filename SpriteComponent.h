#pragma once
#include "Components.h"
#include "TextureManager.h"
#include "ColliderComponent.h"
#include "SDL.h"
#include "Animation.h"
#include <map>

class SpriteComponent : public Component {
private:
	TransformComponent* transform;
	SDL_Texture* texture;
	SDL_Rect srcRect;
	SDL_Rect destRect;

	bool animated = false;
	int frames = 0;
	int speed = 100;

public:
	std::string name;

	int animIndex = 0;
	std::map<const char*, Animation> animations;
	SDL_RendererFlip spriteflip = SDL_FLIP_NONE;

	SpriteComponent() = default;
	SpriteComponent(const char* path) {
		setTex(path);
	}
	SpriteComponent(const char* path, bool isAnimated) {
		animated = isAnimated;
		Animation idle, walk;
		if (name == "player") {
			idle = Animation(0, 10, 300);
			walk = Animation(1, 10, 100);
			
		}
		else {
			idle = Animation(0, 4, 300);
			walk = Animation(1, 8, 100);
		}
		animations.emplace("Idle", idle);
		animations.emplace("Walk", walk);

		Play("Idle");

		setTex(path);
	}

	SpriteComponent(const char* path, bool isAnimated, std::string p_name) {
		name = p_name;
		animated = isAnimated;
		Animation idle, walk, attack;
		if (name == "player") {
			idle = Animation(0, 10, 100);
			walk = Animation(1, 10, 100);
			attack = Animation(2, 4, 100);
		}
		else {
			idle = Animation(0, 3, 300);
			walk = Animation(1, 8, 100);
		}
		animations.emplace("Idle", idle);
		animations.emplace("Walk", walk);
		animations.emplace("Attack", attack);
		Play("Idle");

		setTex(path);
	}
	~SpriteComponent() {
		SDL_DestroyTexture(texture);
	}
	void init() override {

		transform = &entity->getComponent<TransformComponent>();
		srcRect.x = srcRect.y = 0;
		srcRect.w = transform->width;
		srcRect.h = transform->height;
	}

	void setTex(const char* path) {
		texture = TextureManager::LoadTexture(path);
	}

	void update() override {
		if (animated) {
			srcRect.x = srcRect.w * static_cast<int>((SDL_GetTicks() / speed) % frames);
		}
		srcRect.y = animIndex * transform->height;

		destRect.x = static_cast<int>(transform->position.x) - Game::camera.x;
		destRect.y = static_cast<int>(transform->position.y) - Game::camera.y;
		destRect.w = transform->width * transform->scale;
		destRect.h = transform->height * transform->scale;
	}

	void draw() override {
		if (animated) {
			if (animIndex == 2) {
				SDL_Rect tsrcRect = srcRect;
				SDL_Rect tdestRect = destRect;
				tsrcRect.x *= 2;
				tsrcRect.w = 64;
				tdestRect.w *= 2;
				TextureManager::Draw(texture, tsrcRect, tdestRect, spriteflip);
			}
			else {
				TextureManager::Draw(texture, srcRect, destRect, spriteflip);
			}
		}
		else {
			TextureManager::Draw(texture, srcRect, destRect, spriteflip);
		}
	}

	void Play(const char* animName) {
		frames = animations[animName].frames;
		animIndex = animations[animName].index;
		speed = animations[animName].speed;
	}
};
