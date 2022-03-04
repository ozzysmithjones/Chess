#pragma once
#include <array>
#include <queue>
#include "Move.h"

constexpr std::size_t maxKillerMovePly = 5u;
constexpr std::size_t maxKillerMovesPerPly = 2u;

class KillerMoveTable
{

public:
	void PushFront(unsigned int ply, Move move);
	const std::array<Move,maxKillerMovesPerPly>& GetKillerMoves(unsigned int ply) const;

private:
	std::array<std::array<Move, maxKillerMovesPerPly>, maxKillerMovePly> killerMoves{ 0 };
};

