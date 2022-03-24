#pragma once
#include "Piece.h"
#include "Constants.h"


/// <summary>
/// Move packed into an integer.
/// </summary>
using Move = unsigned int;

enum class MoveMask : unsigned int
{
	startPosition	= 0b00000000000000000000111111,
	endPosition		= 0b00000000000000111111000000,
	moveType		= 0b00000000000111000000000000,
	pieceType		= 0b00000000111000000000000000,
	captureType		= 0b00000111000000000000000000,
	promoteType	    = 0b00111000000000000000000000,
	isCheck			= 0b01000000000000000000000000,
	isPin			= 0b10000000000000000000000000,
};


enum class MoveType : unsigned int
{
	normal = 0,
	advancedPawn = 1,
	enpassantHigher = 2,
	enpassantLower = 3,
	castleQueen = 4,
	castleKing = 5,
	promote = 6,
};

void PrintMove(const Move& move);

Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType);
Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType, PieceType captureType);
Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType, PieceType captureType, PieceType promoteType);
Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType, PieceType captureType, PieceType promoteType, bool isCheck, bool isPin);

inline unsigned int GetStartPosition(const Move move) { return move & (unsigned)MoveMask::startPosition; }
inline unsigned int GetEndPosition(const Move move) { return (move & (unsigned)MoveMask::endPosition) >> 6; }
inline MoveType GetMoveType(const Move move) { return (MoveType)((move & (unsigned)MoveMask::moveType) >> 12); }
inline PieceType GetPieceType(const Move move) { return (PieceType)((move & (unsigned)MoveMask::pieceType) >> 15); }
inline PieceType GetCaptureType(const Move move) { return (PieceType)((move & (unsigned)MoveMask::captureType) >> 18); }
inline PieceType GetPromoteType(const Move move) { return (PieceType)((move & (unsigned)MoveMask::promoteType) >> 21); }
inline bool GetIsCheck(const Move move) { return (move & (unsigned)MoveMask::isCheck); }
inline bool GetIsPin(const Move move) { return (move & (unsigned)MoveMask::isPin); }
inline bool GetIsIrreversible(const Move move) { return GetPieceType(move) == PieceType::pawn || GetCaptureType(move) != PieceType::none; }

inline Move SetCheckMove(const Move move) { return (move | ((unsigned)MoveMask::isCheck)); }
inline Move SetPromoteMove(const Move move, PieceType pieceType) { return (move & ~((unsigned)MoveMask::moveType | (unsigned)MoveMask::promoteType)) | (((unsigned int)MoveType::promote << 12) | ((unsigned int)pieceType << 21)); }
