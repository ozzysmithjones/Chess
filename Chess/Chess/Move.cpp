#include "Move.h"
#include <utility>

Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType)
{
	return position | (endPosition << 6) | ((unsigned int)moveType << 18) |
		((unsigned int)pieceType << 21);
}

Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType, PieceType captureType)
{
	return  position | (endPosition << 6) | ((unsigned int)moveType << 18) |
		((unsigned int)pieceType << 21) | ((unsigned int)captureType << 24);
}

Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType, PieceType captureType, PieceType promoteType)
{
	return position | (endPosition << 6) | ((unsigned int)moveType << 18) |
		((unsigned int)pieceType << 21) | ((unsigned int)captureType << 24) | ((unsigned int)promoteType << 27);
}

Move CreateMove(unsigned int position, unsigned int endPosition, unsigned int priority, MoveType moveType, PieceType pieceType, PieceType captureType, PieceType promoteType, bool isCheck, bool isPin)
{
	return position | (endPosition << 6) | (priority << 12) | ((unsigned int)moveType << 18) | ((unsigned int)pieceType << 21) | ((unsigned int)captureType << 24) | ((unsigned int)promoteType << 27)
		| ((unsigned int)isCheck << 28) | ((unsigned int)isPin << 29);
}

void SetPriority(Move& move, unsigned priority)
{
	priority = std::min(63u, priority);
	move |= priority << 12;
}