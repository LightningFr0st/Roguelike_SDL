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

//
// Map implementation
//

Map::Map(const char* mapfile, int mscale, int tilesize) :mapfilePath(mapfile), mapScale(mscale), tileSize(tilesize) {
	scaledSize = mscale * tilesize;
}

Map::~Map() {

}

void Map::LoadMap(int sizeX, int sizeY) {

	int srcX;
	for (int y = 0; y < sizeY; y++) {
		for (int x = 0; x < sizeX; x++) {
			int index = maze_map->getInd(y, x);
			srcX = maze_map->grid[index].indent * tileSize;
			AddTile(srcX, 0, x * scaledSize, y * scaledSize);
			if (maze_map->grid[index].right) {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("terrain", (x + 1) * scaledSize, y * scaledSize, scaledSize / 5, scaledSize, VERTICAL);
				tcol.addGroup(Game::groupColliders);
			}
			if (maze_map->grid[index].bottom) {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("terrain", x * scaledSize, (y + 1) * scaledSize, scaledSize + scaledSize / 5, scaledSize / 5, HORIZONTAL);
				tcol.addGroup(Game::groupColliders);
			}
			if (x == 0) {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("terrain", x * scaledSize, y * scaledSize, scaledSize / 5, scaledSize, VERTICAL);
				tcol.addGroup(Game::groupColliders);
			}
			if (y == 0) {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("terrain", x * scaledSize, y * scaledSize, scaledSize + scaledSize / 5, scaledSize / 5, HORIZONTAL);
				tcol.addGroup(Game::groupColliders);
			}

			if (maze_map->grid[index].lad ) {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("ladder", x * scaledSize, y * scaledSize, scaledSize);
				tcol.addGroup(Game::groupColliders);
			}
			else if (maze_map->grid[index].sword) {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("sword", x * scaledSize + 32, y * scaledSize + 32, 64, 64, NONE);
				tcol.addGroup(Game::groupColliders);
			}
			else if (maze_map->grid[index].map) {
				auto& tcol(manager.addEntity());
				tcol.addComponent<ColliderComponent>("map", x * scaledSize + 32, y * scaledSize + 32, 64, 64, NONE);
				tcol.addGroup(Game::groupColliders);
			}
		}
	}
}

void Map::LoadPath() {
	int srcX = 2 * tileSize;

	for (int i = 0; i < maze_map->escape.size() - 1; i++) {
		int curNode = maze_map->escape[i];
		auto& tile(manager.addEntity());
		tile.addComponent<TileComponent>(srcX, 0, maze_map->grid[curNode].j * scaledSize + 32, maze_map->grid[curNode].i * scaledSize + 32, 32, 2, mapfilePath);
		tile.addGroup(Game::groupPath);
	}
}

void Map::AddTile(int srcX, int srcY, int xpos, int ypos) {
	auto& tile(manager.addEntity());
	tile.addComponent<TileComponent>(srcX, srcY, xpos, ypos, tileSize, mapScale, mapfilePath);
	tile.addGroup(Game::groupMap);
}

//
// Maze implementation
//

Maze::Maze(const int n, const int m) {

	rows = n;
	cols = m;

	visited.resize(rows * cols);
	fill(visited.begin(), visited.end(), false);

	adjList.resize(rows * cols);
	grid.resize(rows * cols);

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
	sword_ind = 46 + (rand() % 45);
	grid[sword_ind].sword = true;
	grid[sword_ind].indent = 0;

	map_ind = 91 + (rand() % 45);
	grid[map_ind].map = true;
	grid[map_ind].indent = 0;
	
	lad_ind = 146 + (rand() % 45);
	grid[lad_ind].lad = true;
	grid[lad_ind].indent = 1;
}

void Maze::BFS() {
	int start = startNode;
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

	int cur = lad_ind;
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
	mino_path.clear();
}

void Maze::unvis() {
	fill(visited.begin(), visited.end() - 1, false);
}

/*void Maze::renderMaze() {

	window = SDL_CreateWindow("Expert System", 0, 50, 800, 952, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Init(SDL_INIT_VIDEO);

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

	SDL_RenderPresent(renderer);
}*/

void Maze::DFS(int mino, int par) {
	if (visited[mino]) {
		//mino_path.push_back(mino);
		return;
	}
	visited[mino] = true;
	mino_path.push_back(mino);
	for (auto next : adjList[mino]) {
		DFS(next, mino);
	}
	mino_path.push_back(par);
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

	lad = false;
	map = false;
	sword = false;
	indent = 0;
}
