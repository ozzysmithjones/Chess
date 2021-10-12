#include "Move.h"

Move::Move() : moveType(MoveType::NORMAL), startPosition(0), endPosition(0)
{
}

Move::Move(MoveType _moveType, unsigned short _startPosition, unsigned short _endPosition)
	: moveType(_moveType), startPosition(_startPosition), endPosition(_endPosition)
{
}

Move::Move(const Move& other) 
	: moveType(other.moveType), startPosition(other.startPosition), endPosition(other.endPosition)
{
}

bool Move::IsPromotion() const
{
	return (unsigned int)moveType >= (unsigned int)MoveType::PROMOTE_QUEEN;
}

bool Move::IsEnPassant() const
{
	return moveType == MoveType::ENPASSANT_HIGHER || moveType == MoveType::ENPASSANT_LOWER;
}

bool Move::IsCastle() const
{
	return moveType == MoveType::CASTLE_HIGHER || moveType == MoveType::CASTLE_LOWER;
}

