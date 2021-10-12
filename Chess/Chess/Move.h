#ifndef MOVE_H
#define MOVE_H

#include "Piece.h"

enum class MoveType
{
	NORMAL,
	ENPASSANT_LOWER,
	ENPASSANT_HIGHER,
	CASTLE_HIGHER,
	CASTLE_LOWER
};

struct Move
{
	MoveType moveType;
	unsigned int startPosition;
	unsigned int endPosition;

	Piece startPiece;
	Piece endPiece;
	Piece capturedPiece;

	bool check;

	Move();
	Move(MoveType moveType, unsigned int startPosition, unsigned int endPosition, Piece startPiece, Piece endPiece, Piece capturedPiece = 0);
};

#endif;

