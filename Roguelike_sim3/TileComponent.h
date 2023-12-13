#pragma once

#include "ECS.h"
#include "SDL.h"

class TileComponent : public Component {
public:

	SDL_Texture* texture;
	SDL_Rect srcRect, dstRect;
	Vector2D position;

	TileComponent() = default;

	~TileComponent() {
		SDL_DestroyTexture(texture);
	}

	TileComponent(int srcX, int srcY, int xpos, int ypos, int tsize, int scale, const char* path) {
		texture = TextureManager::LoadTexture(path);

		position.x = xpos;
		position.y = ypos;

		srcRect.x = srcX;
		srcRect.y = srcY;
		srcRect.w = srcRect.h = tsize;

		dstRect.x = xpos;
		dstRect.y = ypos;
		dstRect.w = dstRect.h = tsize * scale;
	}

	TileComponent(int xpos, int ypos, int tsize, int scale, const char* path) {
		texture = TextureManager::LoadTexture(path);

		position.x = xpos;
		position.y = ypos;

		srcRect.x = 0;
		srcRect.y = 0;
		srcRect.w = srcRect.h = tsize;

		dstRect.x = xpos;
		dstRect.y = ypos;
		dstRect.w = dstRect.h = tsize * scale;
	}

	void update() override {
		dstRect.x = position.x - Game::camera.x;
		dstRect.y = position.y - Game::camera.y;
	}

	void draw() override {
		TextureManager::Draw(texture, srcRect, dstRect, SDL_FLIP_NONE);
	}

};