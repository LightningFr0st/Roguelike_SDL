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
	game->init("Simulation", 900, 50, 972, 972, false);

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

//1 level - black
//2 level - gray
//3 level - green
//4 level - red