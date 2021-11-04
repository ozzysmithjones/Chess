#pragma once
#include <stdlib.h>
#include "Board.h"
#include "TurnState.h"

class ZobristHasher
{
public:

	ZobristHasher();
	~ZobristHasher();

	uint64_t HashForPosition(bool isWhiteTurn, const Board& board, const TurnState& turnState, const unsigned int* whitePlayerPositions, const unsigned int* blackPlayerPositions);
	uint64_t HashForPiece(bool isWhite, PieceType pieceType, unsigned int position);
	uint64_t HashForEnpassant(unsigned int row);
	uint64_t HashForIsWhiteTurn();
	uint64_t HashForCastling(bool white, bool right);

private:

	uint64_t pieceSeeds[64 * 12];
	uint64_t castleSeeds[4];
	uint64_t enpassantSeeds[8];
	uint64_t turnSeed;
};

