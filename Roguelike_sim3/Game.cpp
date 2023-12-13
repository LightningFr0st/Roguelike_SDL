#include "Game.h"

#include "TextureManager.h"
#include "Map.h"
#include "Components.h"
#include "Collision.h"

Map* map; // game map

Manager manager; // game manager for entities

SDL_Renderer* Game::renderer = nullptr; // game renderer

SDL_Rect Game::camera = { 0,0,0,0 };

SDL_Event Game::event;

bool Game::isRunning = false;

auto& player(manager.addEntity()); // creating player entity

//auto& enemies(manager.getGroup(groupEnemies));

Game::Game() {}

Game::~Game() {}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {


	std::cout << "Where are you? : " << std::endl;
	std::cout << "1: There is a lot of scientific stuff here. I'm curtenly in laboratory" << std::endl;
	std::cout << "2: There are many containers and cylinder with rocket fuel. I think I'm in cargo compartment" << std::endl;
	std::cout << "3: I am surrounded with controll panels . It might be a command center" << std::endl;
	std::cout << "4: There is nothing special here. I'm in corridor" << std::endl;

	int roomType;
	std::cin >> roomType;

	std::cout << "What is the color of walls? : " << std::endl;
	std::cout << "1: Black" << std::endl;
	std::cout << "2: Gray" << std::endl;
	std::cout << "3: Green" << std::endl;
	std::cout << "4: Red" << std::endl;

	std::cin >> CurLevel;

	int flags = 0;
	if (fullscreen) {
		flags = SDL_WINDOW_FULLSCREEN;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		std::cout << "Subsystems Initialized" << std::endl;
		window = SDL_CreateWindow(title, xpos, ypos, width, height, flags);
		if (window) {
			std::cout << "Window created" << std::endl;
		}
		renderer = SDL_CreateRenderer(window, -1, 0);
		if (renderer) {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			std::cout << "Renderer created" << std::endl;
		}
		isRunning = true;
	}
	else {
		isRunning = false;
	}

	maze = new Maze(15, 15, "assets/maze.txt", CurLevel);
	maze->generation();

	switch (roomType) {
	case 1:
		maze->startNode = maze->lab_ind;
		break;
	case 2:
		maze->startNode = maze->carg_ind;
		break;
	case 3:
		maze->startNode = maze->com_ind;
		break;
	case 4:
		maze->startNode = 0;
		break;
	}

	maze->BFS(maze->startNode);
	maze->findPath();
	maze->generateMatrix();
	maze->createFile();
	maze->renderMaze();
	maze->clear();

	camera.w = width;
	camera.h = height;

	//initializing map

	map = new Map("assets/terrain_ss.png", 4, 32);
	map->LoadMap("assets/maze.txt", 15, 15, CurLevel);

	//initializing player
	player.addComponent<TransformComponent>(maze->grid[maze->startNode].j * 32 * 4 + 50, maze->grid[maze->startNode].i * 32 * 4 + 50, 32, 32, 2);
	player.addComponent<SpriteComponent>("assets/player_anims.png", true);
	player.addComponent<KeyboardController>();
	player.addComponent<ColliderComponent>("player");
	player.addGroup(groupPlayers);

}

auto& players(manager.getGroup(Game::groupPlayers));
auto& tiles(manager.getGroup(Game::groupMap));
auto& colliders(manager.getGroup(Game::groupColliders));

void Game::handleEvents() {

	SDL_PollEvent(&event);
	switch (event.type)
	{
	case SDL_QUIT:
		isRunning = false;
		break;
	default:
		break;
	}
}

void Game::update() {
	SDL_Rect playerCol = player.getComponent<ColliderComponent>().collider;
	Vector2D playerPos = player.getComponent<TransformComponent>().position;
	colliders = manager.getGroup(Game::groupColliders);
	manager.refresh();
	manager.update();

	for (auto& c : colliders) {
		SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
		if (Collision::AABB(cCol, playerCol)) {
			std::string type = c->getComponent<ColliderComponent>().tag;
			std::cout << "player hit : " << type << '\n';
			if (type == "terrain")
				//player.getComponent<TransformComponent>().position = playerPos
				;
			else if (type == "elev") {

				maze->level -= 2;
				if (maze->level <= 0) {
					isRunning = false;
					this->clean();
					break;
				}

				std::cout << maze->level << std::endl;

				player.getComponent<TransformComponent>().position.x = 50;
				player.getComponent<TransformComponent>().position.y = 50;
				player.getComponent<ColliderComponent>().update();
				playerCol = player.getComponent<ColliderComponent>().collider;

				maze->generation();
				maze->startNode = 0;
				maze->BFS(maze->startNode);
				maze->findPath();
				maze->generateMatrix();
				maze->createFile();
				maze->renderMaze();
				maze->clear();

				manager.clearGroup(groupMap);
				manager.clearGroup(groupColliders);
				map->LoadMap("assets/maze.txt", 15, 15, CurLevel);
				break;
			}
			else if (type == "ladd") {
				maze->level--;

				manager.clearGroup(groupMap);
				manager.clearGroup(groupColliders);

				if (maze->level <= 0) {
					isRunning = false;
					this->clean();
					break;
				}

				std::cout << CurLevel << std::endl;

				player.getComponent<TransformComponent>().position.x = 50;
				player.getComponent<TransformComponent>().position.y = 50;
				player.getComponent<ColliderComponent>().update();
				playerCol = player.getComponent<ColliderComponent>().collider;

				maze->generation();
				maze->startNode = 0;
				maze->BFS(maze->startNode);
				maze->findPath();
				maze->generateMatrix();
				maze->createFile();
				maze->renderMaze();
				maze->clear();
				map->LoadMap("assets/maze.txt", 15, 15, CurLevel);
				break;
			}
		}
	}
	camera.x = player.getComponent<TransformComponent>().position.x - camera.w / 2;
	camera.y = player.getComponent<TransformComponent>().position.y - camera.h / 2;
	if (camera.x < 0)
		camera.x = 0;
	if (camera.y < 0)
		camera.y = 0;
	if (camera.x > camera.w)
		camera.x = camera.w;
	if (camera.y > camera.h)
		camera.y = camera.h;
}

void Game::render() {
	SDL_RenderClear(renderer);
	for (auto& t : tiles) {
		t->draw();
	}

	for (auto& c : colliders) {
		c->draw();
	}

	for (auto& t : players) {
		t->draw();
	}
	/*for (auto& t : enemies) {
		t->draw();
	}*/
	SDL_RenderPresent(renderer);
}

void Game::clean() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	std::cout << "Game cleaned" << std::endl;
}
