#pragma once
#include <array>
#include <vector>
#include "Move.h"

class KillerMoveTable
{

public:
	void Push_front(unsigned int ply, Move move);
	const Move* GetKillerMoves(unsigned int ply, size_t& length) const;

private:

	static const size_t maxKillerMovePly = 5;
	static const size_t maxKillerMovesPerPly = 2;
	Move killerMoves[maxKillerMovePly][maxKillerMovesPerPly]{ 0 };
};

