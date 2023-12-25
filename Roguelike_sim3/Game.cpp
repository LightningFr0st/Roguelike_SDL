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

Entity& enemy(manager.addEntity());

auto& sword(manager.addEntity());

auto& lab_map(manager.addEntity());


bool sword_taken = false;
bool map_taken = false;
bool enemykilled = false;

int golem_ind = 112;

bool res = false;

Game::Game() {}

Game::~Game() {}

void Game::init(const char* title, int xpos, int ypos, int width, int height, bool fullscreen) {

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

	camera.w = width;
	camera.h = height;

	map = new Map("assets/ext_terrain.png", 4, 32);
	map->maze_map = new Maze(15, 15);
	map->maze_map->generation();
	map->maze_map->startNode = map->maze_map->map_ind;
	map->maze_map->BFS();
	map->maze_map->findPath();
	map->maze_map->unvis();
	map->maze_map->DFS(golem_ind, golem_ind);

	//initializing map

	map->LoadMap(15, 15);

	//initializing player
	player.addComponent<TransformComponent>(50, 50, 32, 37, 2);
	player.getComponent<TransformComponent>().speed = 10;
	player.addComponent<SpriteComponent>("assets/ext_animations.png", true, "player");
	player.addComponent<ColliderComponent>("player");
	player.addComponent<KeyboardController>();

	player.addGroup(groupPlayers);

	enemy.addComponent<TransformComponent>(7 * 128 + 50, 7 * 128 + 50, 32, 32, 2);
	enemy.addComponent<SpriteComponent>("assets/golem.png", true, "golem");
	enemy.addComponent<ColliderComponent>("enemy");
	enemy.addComponent<AIComponent>();
	enemy.getComponent<AIComponent>().init_path(map->maze_map->mino_path);
	enemy.addGroup(groupEnemies);


	/*sword.addComponent<TransformComponent>(60, 60, 32, 32, 1);
	sword.addComponent<SpriteComponent>("assets/sword.png", false);
	sword.addComponent<ColliderComponent>("sword");
	sword.addGroup(groupColliders);

	lab_map.addComponent<TransformComponent>(188, 60, 32, 32, 1);
	lab_map.addComponent<SpriteComponent>("assets/map.png", false);
	lab_map.addComponent<ColliderComponent>("map");
	lab_map.addGroup(groupColliders);*/
}

auto& players(manager.getGroup(Game::groupPlayers));
auto& tiles(manager.getGroup(Game::groupMap));
auto& colliders(manager.getGroup(Game::groupColliders));
auto& path(manager.getGroup(Game::groupPath));
auto& enemies(manager.getGroup(Game::groupEnemies));

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
	enemies = manager.getGroup(Game::groupEnemies);
	manager.refresh();
	manager.update();
	res = false;

	for (auto& c : colliders) {
		SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
		if (Collision::AABB(cCol, playerCol)) {
			std::string type = c->getComponent<ColliderComponent>().tag;
			std::cout << "player hit : " << type << '\n';
			if (type == "ladder") {
				restart();
				res = true;
				break;
			}
			else if (type == "terrain") {
				//player.getComponent<TransformComponent>().position = playerPos;
			}
			else if (!map_taken && type == "map") {
				map_taken = true;
				c->getComponent<ColliderComponent>().visible = false;
				map->LoadPath();
			}
			else if (type == "sword") {
				sword_taken = true;
				c->getComponent<ColliderComponent>().visible = false;
			}
		}
	}
	if (!res && !enemykilled) {
		for (auto& c : enemies) {
			SDL_Rect cCol = c->getComponent<ColliderComponent>().collider;
			if (Collision::AABB(cCol, playerCol)) {
				std::string type = c->getComponent<ColliderComponent>().tag;
				std::cout << "player hit : " << type << '\n';
				if (type == "enemy") {
					if (sword_taken) {
						player.getComponent<SpriteComponent>().Play("Attack");
						enemy.getComponent<AIComponent>().killed();
						enemykilled = true;
					}
					else {
						Lose();
						res = true;
					}
				}
			}
		}
	}	
	camera.x = player.getComponent<TransformComponent>().position.x - camera.w / 2;
	camera.y = player.getComponent<TransformComponent>().position.y - camera.h / 2;
	if (camera.x < 0)
		camera.x = 0;
	if (camera.y < 0)
		camera.y = 0;
	if (camera.x > camera.w / 6 - 295)
		camera.x = camera.w / 6 - 295;
	if (camera.y > camera.h - 220)
		camera.y = camera.h - 220;
}

void Game::restart() {
	
	clearmap();

	sword_taken = false;
	map_taken = false;
	enemykilled = false;
	//initializing player
	player.getComponent<TransformComponent>().position.x = 50;
	player.getComponent<TransformComponent>().position.y = 50;
	player.getComponent<TransformComponent>().velocity.x = 0;
	player.getComponent<TransformComponent>().velocity.y = 0;

	//initializing golem

	enemy.getComponent<AIComponent>().killed();

	manager.refresh();

	//initializing map
	map->maze_map->clear();
	map->maze_map->generation();
	map->maze_map->startNode = map->maze_map->map_ind;
	map->maze_map->BFS();
	map->maze_map->findPath();
	map->maze_map->unvis();
	map->maze_map->DFS(golem_ind, golem_ind);
	map->LoadMap(15, 15);

	enemy.getComponent<AIComponent>().init_path(map->maze_map->mino_path);

	manager.update();
}

void Game::clearmap() {
	sword_taken = false;
	map_taken = false;
	enemykilled = false;
	for (auto t : tiles) {
		t->destroy();
	}
	for (auto t : path) {
		t->destroy();
	}
	for (auto t : colliders) {
		t->destroy();
	}
}

void Game::render() {
	SDL_RenderClear(renderer);
	for (auto& t : tiles) {
		t->draw();
	}

	for (auto& c : colliders) {
		c->draw();
	}
	if (map_taken) {
		for (auto& c : path) {
			c->draw();
		}
	}
	if (!enemykilled) {
		for (auto& t : enemies) {
			t->draw();
		}
	}
	
	for (auto& t : players) {
		t->draw();
	}

	SDL_RenderPresent(renderer);
}

void Game::clean() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	std::cout << "Game cleaned" << std::endl;
}


void Game::Lose() {
	std::cout << "YOU LOSE\n";
	TTF_Init();
	char* text1 = const_cast<char*>("You loose");
	char* text2 = const_cast<char*>("Press R to restart level");
	TTF_Font* outFont = TTF_OpenFont("C:/Users/Denis/AppData/Local/Microsoft/Windows/Fonts/8bitOperatorPlus8-Regular.ttf", 32);
	SDL_Color White = { 255, 0, 0, SDL_ALPHA_OPAQUE };
	SDL_Surface* surfaceText = TTF_RenderText_Solid(outFont, text1, White);
	SDL_Texture* textureText1 = SDL_CreateTextureFromSurface(renderer, surfaceText);
	SDL_FreeSurface(surfaceText);
	surfaceText = TTF_RenderText_Solid(outFont, text2, White);
	SDL_Texture* textureText2 = SDL_CreateTextureFromSurface(renderer, surfaceText);
	SDL_FreeSurface(surfaceText);
	SDL_Rect textRect;
	
	textRect.x = 800;
	textRect.y = 540;
	textRect.w = 300;
	textRect.h = 100;

	SDL_RenderCopy(renderer, textureText1, NULL, &textRect);

	textRect.x = 800;
	textRect.y = 740;
	textRect.w = 300;
	textRect.h = 100;

	SDL_RenderCopy(renderer, textureText2, NULL, &textRect);

	SDL_RenderPresent(renderer);
	
	SDL_Delay(2000);
	restart();
}