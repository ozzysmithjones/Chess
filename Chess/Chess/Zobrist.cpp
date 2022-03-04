#include <algorithm>
#include "Random.h"
#include "Zobrist.h"
#include "GameState.h"

Zobrist::Zobrist()
{
    Random random;

    for (int i = 0; i < 4; i++)
    {
        castleSeeds[i] = random.GetRandomInt<uint64_t>();
    }

    for (int i = 0; i < 8; i++)
    {
        enpassantSeeds[i] = random.GetRandomInt<uint64_t>();
    }

    for (int i = 0; i < 12; i++)
    {
        for (int j = 0; j < 64; j++)
        {
            pieceSeeds[i][j] = random.GetRandomInt<uint64_t>();
        }
    }

    turnSeed = random.GetRandomInt<uint64_t>();
}

uint64_t Zobrist::CalculateZobristKey(bool isWhiteTurn, const GameState& gameState)
{
    uint64_t key = 0;

    
    if (!isWhiteTurn)
    {
        key ^= turnSeed;
    }

    for (int i = 0; i < 4; i++)
    {
        if (gameState.castlingRights & (1 << i))
        {
            key ^= castleSeeds[i];
        }
    }
  
    if (gameState.enpassantSquare != NO_ENPASSANT)
    {
        key ^= enpassantSeeds[gameState.enpassantSquare >> 3];
    }

    for (int i = 0; i < 6; i++)
    {
        //For white
        for (uint64_t piece = (gameState.pieces[i] & gameState.occupancy[1]); piece != 0;)
        {
            unsigned square = GetLeastIndex(piece);
            key ^= pieceSeeds[i][square];
            PopBit(piece, square);
        }

        //For black
        for (uint64_t piece = (gameState.pieces[i] & gameState.occupancy[0]); piece != 0;)
        {
            unsigned square = GetLeastIndex(piece);
            key ^= pieceSeeds[6 + i][square];
            PopBit(piece, square);
        }
    }

    return key;
}

uint64_t Zobrist::GetPieceSeed(unsigned int type, unsigned int square, bool isWhite) const
{
    return pieceSeeds[isWhite ? type : (type + 6)][square];
}

uint64_t Zobrist::GetCastleSeed(bool kingSide, bool isWhite) const
{
    return castleSeeds[((uint8_t)(!isWhite) << 1) + (uint8_t)kingSide];
}

uint64_t Zobrist::GetEnpassantSeed(unsigned int file) const
{
    return enpassantSeeds[file];
}

uint64_t Zobrist::GetTurnSeed() const
{
    return turnSeed;
}
