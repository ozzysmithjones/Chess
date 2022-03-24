#pragma once
#include <stdint.h>
struct GameState;

class Zobrist
{
public:

	Zobrist();
	uint64_t CalculateZobristKey(bool isWhiteTurn, const GameState& gameState);

	uint64_t GetPieceSeed(unsigned int type, unsigned int square, bool isWhite) const;
	uint64_t GetCastleSeed(bool kingSide, bool isWhite) const;
	uint64_t GetEnpassantSeed(unsigned int file) const;
	uint64_t GetTurnSeed() const;

private:
	uint64_t pieceSeeds[12][64];
	uint64_t castleSeeds[4];
	uint64_t enpassantSeeds[8];
	uint64_t turnSeed;
};

