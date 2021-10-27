#pragma once
#include "Piece.h"

inline unsigned int ToIndex(unsigned int x, unsigned int y) { return (y << 3) + x; }

struct Board
{
public:

	static const int HEIGHT = 8;
	static const int WIDTH = 8;
	static const int MIN_ROW_INDEX = 0;
	static const int MIN_COL_INDEX = 0;
	static const int MAX_ROW_INDEX = HEIGHT;
	static const int MAX_COL_INDEX = WIDTH;

	inline bool InBounds(int x, int y) const { return x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT; }
	inline bool IsPieceAt(unsigned int x, unsigned int y) { return grid[(y << 3) + x] != 0; }
	inline Piece& At(unsigned int x, unsigned int y) { return grid[(y << 3) + x]; }
	inline const Piece& C_At(unsigned int x, unsigned int y) const { return grid[(y << 3) + x]; }
	inline Piece& operator[](unsigned int index) { return grid[index]; }
	inline const Piece& operator[](unsigned int index) const { return grid[index]; }
	inline const Piece& C_Index(unsigned int index) const { return grid[index]; }

private:

	Piece grid[64]{ 0 };
};

