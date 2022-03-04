#include "Piece.h"

Piece CreatePiece(bool white, PieceType type, unsigned int id)
{
	return 1u | ((unsigned int)white << 1) | ((unsigned int)type << 2) | (id << 5) | (pieceScores[(unsigned int)type - 1] << 9);
}
