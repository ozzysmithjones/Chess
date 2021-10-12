#ifndef MOVE_H
#define MOVE_H

#include "Piece.h"

enum class MoveType : unsigned int
{
	NORMAL,
	ENPASSANT_LOWER,
	ENPASSANT_HIGHER,
	CASTLE_HIGHER,
	CASTLE_LOWER,

	PROMOTE_QUEEN,
	PROMOTE_ROOK,
	PROMOTE_BISHOP,
	PROMOTE_KNIGHT,
};

struct Move
{
	unsigned int startPosition : 6;
	unsigned int endPosition : 6;
	MoveType moveType : 4;

	Move();
	Move(MoveType moveType,unsigned short startPosition, unsigned short endPosition);
	Move(const Move& other);

	//Move operator=(const Move& other);

	bool IsPromotion() const;
	bool IsEnPassant() const;
	bool IsCastle() const;
};

#endif;

