#include "Game.h"

Game* game = nullptr;

int main(int argc, char* argv[]) {
	//VSYNC section
	const int FPS = 60;
	const int frameDelay = 1000 / FPS;

	//variables to measure elapsed time
	Uint32 frameStart;
	int frameTime;

	//creating and initializing game

	game = new Game;
	game->init("Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, false);

	//main game loop
	while (game->running()) {
		frameStart = SDL_GetTicks();

		//updating and rendering game
		game->handleEvents();
		game->update();
		game->render();

		//delaying next frame
		frameTime = SDL_GetTicks() - frameStart;
		if (frameTime < frameDelay) {
			SDL_Delay(frameDelay - frameTime);
		}
	}
	//cleanzing game
	game->clean();
	std::cout << "You Win!";
	return 0;
}
