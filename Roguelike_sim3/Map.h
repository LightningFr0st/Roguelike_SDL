#pragma once
#include "ECS.h"
#include "SDL.h"
#include "SDL_image.h"
#include <vector>
#include <string>
#include <fstream>
#include <tuple>

class Maze;

struct Cell {
	Maze* maze;

	//index
	int i;
	int j;

	//walls
	bool top = true;
	bool left = true;
	bool bottom = true;
	bool right = true;

	//special 
	bool lad = false;
	bool map = false;
	bool sword = false;
	int indent = 0;

	Cell() = default;
	Cell(int p_i, int p_j);
	Cell* getNext(std::vector<Cell>& p_grid);
	void null();
};

class Maze {
private:
	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	int cols;
	int rows;

	//vectors for calcs
	std::vector<bool> visited; //+

	std::vector<int> distance; //+
	std::vector<std::vector<int>> adjList; //+

public:
	int level;
	int startNode;
	int lad_ind;
	int map_ind;
	int sword_ind;

	std::vector<Cell> grid;
	std::vector<int> escape;
	std::vector<int> mino_path;

	Maze(const int n, const int m);
	~Maze();

	int getRows() { return rows; };
	int getCols() { return cols; };
	int getInd(int i, int j);
	bool visCheck(int i, int j);
	void remWalls(Cell* prev, Cell* next);

	void generation();
	void BFS();
	void DFS(int mino, int par);
	void findPath();
	void clear();
	void unvis();
	void renderMaze();
};

class Map {
public:
	Map(const char* mapfile, int mscale, int tilesize);
	~Map();
	
	void LoadMap(int sizeX, int sizeY);
	void LoadPath();
	void AddTile(int srcX, int srcY, int xpos, int ypos);
	Maze* maze_map;

private:
	const char* mapfilePath;
	int mapScale;
	int tileSize;
	int scaledSize;
	int level;
};

