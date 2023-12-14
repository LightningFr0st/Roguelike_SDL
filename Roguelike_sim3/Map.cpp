#include "Map.h"
#include "Game.h"
#include "ECS.h"
#include "SDL_ttf.h"
#include "Components.h"

#include <fstream>
#include <stack>
#include <queue>
#include <algorithm>
#include <ctime>

extern Manager manager;

Map::Map(const char* mapfile, int mscale, int tilesize) :mapfilePath(mapfile), mapScale(mscale), tileSize(tilesize) {
	scaledSize = mscale * tilesize;
}

Map::~Map() {

}

void Map::LoadMap(std::string path, int sizeX, int sizeY, int p_level) {
	level = p_level;
	char c;
	std::fstream mapFile;
	mapFile.open(path);

	int srcX, srcY;
	for (int y = 0; y < sizeY; y++) {
		for (int x = 0; x < sizeX; x++) {
			int index = maze_map.getInd(y, x);
			srcX = maze_map.grid[index].indent * tileSize;
			AddTile(srcX, 0, x * scaledSize, y * scaledSize);
			if (maze_map.grid[index].indent == 1) {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>
			}
		}
	}

	for (int y = 0; y < sizeY; y++) {
		for (int x = 0; x < sizeX; x++) {
			mapFile.get(c);
			srcX = atoi(&c) * tileSize;
			AddTile(srcX, 0, x * scaledSize, y * scaledSize);
			if (srcX / tileSize == 1) {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("elev", x * scaledSize, y * scaledSize, scaledSize);
				tcol.addGroup(Game::groupColliders);
			}
			mapFile.ignore();
		}
	}

	mapFile.ignore();

	for (int y = -1; y < sizeY; y++) {
		for (int x = -1; x < sizeX; x++) {
			mapFile.get(c);
			if (c == '1') {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("terrain", (x + 1) * scaledSize, y * scaledSize, scaledSize / 5, scaledSize, level);
				tcol.addGroup(Game::groupColliders);
			}
			mapFile.get(c);
			if (c == '1') {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("terrain", x * scaledSize, (y + 1) * scaledSize, scaledSize + scaledSize / 5, scaledSize / 5, level);
				tcol.addGroup(Game::groupColliders);
			}
			mapFile.ignore();
		}
	}

	mapFile.close();
}

void Map::AddTile(int srcX, int srcY, int xpos, int ypos) {
	auto& tile(manager.addEntity());
	tile.addComponent<TileComponent>(srcX, srcY, xpos, ypos, tileSize, mapScale, mapfilePath);
	tile.addGroup(Game::groupMap);
}

Maze::Maze(const int n, const int m, std::string path, int p_level) {

	window = SDL_CreateWindow("Expert System", 0, 50, 800, 952, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Init(SDL_INIT_VIDEO);
	level = p_level;
	rows = n;
	cols = m;
	mazePath = path;
	visited.resize(rows * cols);
	fill(visited.begin(), visited.end(), false);

	adjList.resize(rows * cols);
	grid.resize(rows * cols);

	escape.resize(2);
	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			grid[this->getInd(i, j)].maze = this;
			grid[this->getInd(i, j)].i = i;
			grid[this->getInd(i, j)].j = j;
		}
	}
}

Maze::~Maze() {}


int Maze::getInd(int i, int j) {
	if (i != -1 && i != rows + 1 && j != -1 && j != cols + 1) {
		return i * cols + j;
	}
	else {
		return -1;
	}
}


bool Maze::visCheck(int i, int j) {
	return visited[this->getInd(i, j)];
}

void Maze::remWalls(Cell* cur, Cell* next) {
	int ind_a = getInd(cur->i, cur->j);
	int ind_b = getInd(next->i, next->j);

	adjList[ind_a].push_back(ind_b);
	adjList[ind_b].push_back(ind_a);

	if (ind_b - ind_a == 1) {
		next->left = false;
		cur->right = false;
	}
	else if (ind_b - ind_a == -1) {
		cur->left = false;
		next->right = false;
	}
	else if (ind_b - ind_a > 1) {
		cur->bottom = false;
		next->top = false;
	}
	else if (ind_b - ind_a < -1) {
		cur->top = false;
		next->bottom = false;
	}
}

void Maze::generation() {
	srand(time(NULL));

	// maze-generating algorithm
	
	Cell* cur = &grid[0];
	visited[0] = true;
	std::stack<Cell*> stack_cell;
	stack_cell.push(cur);
	
	while (!stack_cell.empty()) {
		cur = stack_cell.top();
		stack_cell.pop();
		Cell* next = cur->getNext(grid);
		if (next != nullptr) {
			stack_cell.push(cur);
			remWalls(cur, next);
			visited[getInd(next->i, next->j)] = true;
			stack_cell.push(next);
		}
	}

	elev_ind = 46 + (rand() % 45);
	carg_ind = 91 + (rand() % 45);
	com_ind = 136 + (rand() % 45);

	grid[elev_ind].elev = true;
	grid[elev_ind].indent = 1;

	grid[lab_ind].lab = true;
	grid[lab_ind].indent = 3;
	
	grid[carg_ind].carg = true;
	grid[carg_ind].indent = 4;
	
	grid[com_ind].com = true;
	grid[com_ind].indent = 5;
	
}

void Maze::BFS(int start) {
	fill(visited.begin(), visited.end() - 1, false);
	std::queue<int> order;

	distance.resize(rows * cols);
	visited[start] = true;
	distance[start] = 0;
	order.push(start);

	while (!order.empty()) {
		int cur = order.front();
		order.pop();
		for (std::vector<int>::iterator it = adjList[cur].begin(); it != adjList[cur].end(); it++) {
			if (visited[*it]) continue;
			visited[*it] = true;
			distance[*it] = distance[cur] + 1;
			order.push(*it);
		}
	}
}

void Maze::findPath() {

	int cur = elev_ind;
	while (cur != startNode) {
		for (std::vector<int>::iterator it = adjList[cur].begin(); it != adjList[cur].end(); it++) {
			if (distance[*it] - distance[cur] == -1) {
				escape.push_back(cur);
				cur = *it;
				break;
			}
		}
	}
	escape.push_back(cur);
	reverse(escape.begin(), escape.end());
}


void Maze::clear() {
	for (int i = 0; i < rows * cols; i++) {
		grid[i].null();
		visited[i] = false;
		distance[i] = 0;
		adjList[i].clear();
	}
	escape.clear();
}

void Maze::renderMaze() {

	SDL_Rect frame;
	frame.x = 25;
	frame.y = 25;
	frame.w = 750;
	frame.h = 750;

	SDL_Rect cell;
	cell.w = 40;
	cell.h = 40;

	int width = 50;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

	// Celar the screen

	SDL_RenderClear(renderer);

	// Draw the maze

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	SDL_RenderDrawRect(renderer, &frame);

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < cols; j++) {
			int ind = getInd(i, j);

			if (grid[ind].right) {
				SDL_RenderDrawLine(renderer, (j + 1) * width + 25, i * width + 25,
					(j + 1) * width + 25, (i + 1) * width + 25);
			}

			if (grid[ind].bottom) {
				SDL_RenderDrawLine(renderer, j * width + 25, (i + 1) * width + 25,
					(j + 1) * width + 25, (i + 1) * width + 25);
			}

			if (grid[ind].elev || grid[ind].lad) {
				cell.x = (j * 50) + 30;
				cell.y = (i * 50) + 30;
				SDL_RenderDrawRect(renderer, &cell);
			}
		}
	}

	//Draw the path

	SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE);
	for (int i = 1; i < escape.size(); i++) {
		int x1 = grid[escape[i - 1]].j * width + 25, y1 = grid[escape[i - 1]].i * width + 25;
		int x2 = grid[escape[i]].j * width + 25, y2 = grid[escape[i]].i * width + 25;
		x1 = (x1 + x1 + width) / 2;
		y1 = (y1 + y1 + width) / 2;
		x2 = (x2 + x2 + width) / 2;
		y2 = (y2 + y2 + width) / 2;
		SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
	TTF_Init();
	char text[10] = "Level : ";
	char char_level = static_cast<char>(level + 48);
	level--;
	TTF_Font* outFont = TTF_OpenFont("C:/Users/Denis/AppData/Local/Microsoft/Windows/Fonts/8bitOperatorPlus8-Regular.ttf", 32);
	text[8] = char_level;
	SDL_Color White = { 255, 255, 255, SDL_ALPHA_OPAQUE };
	SDL_Surface* surfaceText = TTF_RenderText_Solid(outFont, text, White);
	SDL_Texture* textureText = SDL_CreateTextureFromSurface(renderer, surfaceText);
	SDL_FreeSurface(surfaceText);

	SDL_Rect textRect;
	textRect.x = 25;
	textRect.y = 800;
	textRect.w = 300;
	textRect.h = 100;

	SDL_RenderCopy(renderer, textureText, NULL, &textRect);

	SDL_RenderPresent(renderer);
}


///
/// Cell implementation
///


Cell::Cell(int p_i, int p_j) {
	i = p_i;
	j = p_j;
}

Cell* Cell::getNext(std::vector<Cell>& p_grid) {

	std::vector<Cell*> neighbours;

	// top cell
	if (i - 1 != -1 && !maze->visCheck(i - 1, j)) {
		neighbours.push_back(&p_grid[maze->getInd(i - 1, j)]);
	}

	// left cell
	if (j - 1 != -1 && !maze->visCheck(i, j - 1)) {
		neighbours.push_back(&p_grid[maze->getInd(i, j - 1)]);
	}

	// right cell
	if (j + 1 != maze->getCols() && !maze->visCheck(i, j + 1)) {
		neighbours.push_back(&p_grid[maze->getInd(i, j + 1)]);
	}

	// bottom cell
	if (i + 1 != maze->getRows() && !maze->visCheck(i + 1, j)) {
		neighbours.push_back(&p_grid[maze->getInd(i + 1, j)]);
	}

	if (neighbours.size() > 0) {
		int r = rand() % neighbours.size();
		return neighbours[r];
	}
	else {
		return nullptr;
	}
}

void Cell::null() {
	top = true;
	left = true;
	bottom = true;
	right = true;

	elev = false;
	lad = false;
	lab = false;
	carg = false;
	com = false;
	indent = 0;
}
