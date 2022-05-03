#pragma once
#include "Move.h"
#include "BitboardManipulation.h"

enum Castling : unsigned
{
	whiteShort  = 1,
	whiteLong   = 2,
	blackShort  = 4,
	blackLong   = 8,
};

constexpr auto NO_ENPASSANT = 64;

struct GameState
{
	Move move = 0;									 //The move made this turn.
	uint64_t pieces[6]{ 0ull };                      // pawns, bishops, knights, rooks, queen, king
	uint64_t occupancy[2]{ 0ull };                   // black, white
	uint64_t zobristKey{ 0ull };
	uint32_t enpassantSquare = NO_ENPASSANT;		 // from 0 - 63 (64 being no enpassant possibility)
	uint32_t castlingRights;						 // 0 means that castling is legal, set bit represents castling is not legal. 0000 - 1111 (for both sides)
	bool inCheck = false;
};

