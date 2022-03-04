#include "Move.h"

Move CreateMove(unsigned int startPosition, unsigned int endPosition, MoveType moveType, PieceType pieceType, PieceType captureType, bool isCheck, bool isPin, PieceType promoteType)
{
	return startPosition | (endPosition << 6) | ((unsigned int)moveType << 12) |
		((unsigned int)captureType << 15) | ((unsigned int)promoteType << 18) | ((unsigned int)pieceType << 21)
		| ((unsigned int)isCheck << 24) | ((unsigned int)isPin << 25);
}
