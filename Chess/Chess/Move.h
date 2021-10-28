#ifndef MOVE_H
#define MOVE_H
#include "Piece.h"

typedef unsigned int Move;

#pragma region 

enum MoveMask : unsigned int
{
	START_POSITION =	0b000000000000000111111,
	END_POSITION =		0b000000000111111000000,
	MOVE_TYPE =			0b000000111000000000000,
	CAPTURE_TYPE =		0b000111000000000000000,
	PROMOTE_TYPE =		0b111000000000000000000,
};

enum class MoveType : unsigned int
{
	NORMAL = 0,
	ADVANCED_PAWN = 1,
	ENPASSANT_HIGHER = 2,
	ENPASSANT_LOWER = 3,
	CASTLE_HIGHER = 4,
	CASTLE_LOWER = 5,
	PROMOTION = 6,
};

Move				CreateMove(unsigned int startPosition, unsigned int endPosition, MoveType moveType,PieceType captureType = PieceType::NONE, PieceType promoteType = PieceType::NONE);
inline unsigned int GetStartPos(const Move move) { return move & MoveMask::START_POSITION; }
inline unsigned int GetEndPos(const Move move) { return (move & MoveMask::END_POSITION) >> 6; }
inline MoveType     GetMoveType(const Move move) { return (MoveType)((move & MoveMask::MOVE_TYPE) >> 12); }
inline PieceType	GetCaptureType(const Move move) { return (PieceType)((move & MoveMask::CAPTURE_TYPE) >> 15); }
inline PieceType	GetPromoteType(const Move move) { return (PieceType)((move & MoveMask::PROMOTE_TYPE) >> 18); }
inline Move			SetPromoteMove(const Move move, PieceType pieceType) { return (move & ~(MoveMask::MOVE_TYPE | MoveMask::PROMOTE_TYPE))  | (((unsigned int)MoveType::PROMOTION << 12) | ((unsigned int)pieceType << 18)); }


#pragma endregion Move



#endif;



