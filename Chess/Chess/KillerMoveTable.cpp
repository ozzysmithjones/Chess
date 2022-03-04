#include "KillerMoveTable.h"

void KillerMoveTable::PushFront(unsigned int ply, Move move)
{
	for (std::size_t i = 0; i < killerMoves[ply].size()-1; i++)
	{
		killerMoves[ply][i + 1] = killerMoves[ply][i];
	}

	killerMoves[ply][0] = move;
}

const std::array<Move, maxKillerMovesPerPly>& KillerMoveTable::GetKillerMoves(unsigned int ply) const
{
	return killerMoves[ply];
}
