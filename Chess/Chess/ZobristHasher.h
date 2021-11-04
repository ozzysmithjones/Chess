#pragma once
#include "Board.h"
#include "TurnState.h"

class ZobristHasher
{
public:

	ZobristHasher();
	~ZobristHasher();

	unsigned long long HashForPosition(bool isWhiteTurn, const Board& board, const TurnState& turnState, const unsigned int* whitePlayerPositions, const unsigned int* blackPlayerPositions);
	unsigned long long HashForPiece(bool isWhite, PieceType pieceType, unsigned int position);
	unsigned long long HashForEnpassant(unsigned int row);
	unsigned long long HashForIsWhiteTurn();
	unsigned long long HashForCastling(bool white, bool right);

private:

	unsigned long long pieceSeeds[64 * 12];
	unsigned long long castleSeeds[4];
	unsigned long long enpassantSeeds[8];
	unsigned long long turnSeed;
};

