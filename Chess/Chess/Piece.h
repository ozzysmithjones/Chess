#pragma once
const unsigned int pieceScores[6] = { 1,3,3,5,9,15 };

typedef unsigned int Piece;
enum PieceMask : unsigned int
{
	VALID =		0b00000000000000001,
	WHITE =		0b00000000000000010,
	TYPE =		0b00000000000011100,
	ID =		0b00000000111100000,
	SCORE =		0b00001111000000000,
};

enum class PieceType : unsigned int
{
	NONE = 0u,
	PAWN = 1u,
	KNIGHT = 2u,
	BISHOP = 3u,
	ROOK = 4u,
	QUEEN = 5u,
	KING = 6u,
};


Piece CreatePiece(bool white, PieceType type, unsigned int id);
inline bool IsValid(const Piece piece) { return piece & PieceMask::VALID; }
inline bool IsWhite(const Piece piece) { return piece & PieceMask::WHITE; }
inline PieceType GetType(const Piece piece) { return (PieceType)((piece & PieceMask::TYPE) >> 2); }
inline unsigned int GetId(const Piece piece) { return (piece & PieceMask::ID) >> 5; }
inline unsigned int GetScore(const Piece piece) { return (piece & PieceMask::SCORE) >> 9; }
inline unsigned int GetScore(const PieceType piece) { return pieceScores[(unsigned int)piece]; }
inline void SetType(Piece& piece, PieceType pieceType) { piece = (piece | ((unsigned int)pieceType << 2) | (pieceScores[(unsigned int)pieceType] << 9)); }
