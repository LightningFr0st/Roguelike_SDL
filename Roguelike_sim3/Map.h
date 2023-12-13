#pragma once
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
	bool elev = false;
	bool lad = false;
	bool lab = false;
	bool carg = false;
	bool com = false;

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

	std::string mazePath;

	//vectors for calcs
	std::vector<bool> visited; //+

	std::vector<int> distance; //+
	std::vector<std::vector<int>> adjList; //+

public:

	int level;

	int startNode;

	int elev_ind;
	int lad_ind;
	int carg_ind;
	int com_ind;
	int lab_ind;


	std::vector<Cell> grid;
	std::vector<int> escape;

	Maze() {};

	Maze(const int n, const int m, std::string path, int p_level);
	
	~Maze();

	int getRows() { return rows; };
	int getCols() { return cols; };

	int getInd(int i, int j);
	
	bool visCheck(int i, int j);
	
	void remWalls(Cell* prev, Cell* next);
	
	void generation();
	
	void BFS(int start);
	
	void findPath();

	void clear();

	void renderMaze();
};

class Map {
public:
	Map(const char* mapfile, int mscale, int tilesize);
	~Map();
	
	void LoadMap(std::string path, int sizeX, int sizeY, int p_level);
	void AddTile(int srcX, int srcY, int xpos, int ypos);

private:
	const char* mapfilePath;
	int mapScale;
	int tileSize;
	int scaledSize;
	int level;
	Maze maze_map;
};

