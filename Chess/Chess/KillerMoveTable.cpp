#include "KillerMoveTable.h"



void KillerMoveTable::Push_front(unsigned int ply, Move move)
{
    for (unsigned int i = 1; i < maxKillerMovesPerPly-1; i++)
    {
        killerMoves[ply][i + 1] = killerMoves[ply][i];
    }

    killerMoves[ply][0] = move;
}

const Move* KillerMoveTable::GetKillerMoves(unsigned int ply, size_t& length) const
{
    length = maxKillerMovesPerPly;
    return killerMoves[ply];
}
