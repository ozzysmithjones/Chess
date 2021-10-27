#include "Move.h"

Move CreateMove(unsigned int startPosition, unsigned int endPosition, MoveType moveType,PieceType captureType,PieceType promoteType)
{
	return startPosition | (endPosition << 6) | ((unsigned int)moveType << 12) | 
		((unsigned int)captureType << 15) | ((unsigned int) promoteType << 18);
}
