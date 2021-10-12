#include "Move.h"

Move::Move()
{
	moveType = MoveType::NORMAL;
	startPosition = 0;
	endPosition = 0;
	capturedPiece = 0;
	startPiece = 0;
	endPiece = 0;
}

Move::Move(MoveType _moveType, unsigned int _startPosition, unsigned int _endPosition, Piece _startPiece, Piece _endPiece, Piece _capturedPiece)
{
	moveType = _moveType;
	startPosition = _startPosition;
	endPosition = _endPosition;
	startPiece = _startPiece;
	endPiece = _endPiece;
	capturedPiece = _capturedPiece;
}