#include <stdlib.h>
#include <limits>
#include <random>
#include "ZobristHasher.h"

typedef std::mt19937 RNG;
const uint32_t seed_val = 137;

ZobristHasher::ZobristHasher()
{
    RNG rng;
    rng.seed(seed_val);
    std::uniform_int_distribution<uint64_t> dist;

    for (unsigned int i = 0; i < 64 * 12; i++)
    {
        pieceSeeds[i] = (unsigned long long)dist(rng);//rand() % std::numeric_limits<unsigned long long>::max();
    }

    for (unsigned int i = 0; i < 4; i++)
    {
        castleSeeds[i] = (unsigned long long)dist(rng);
    }

    for (unsigned int i = 0; i < 8; i++)
    {
        enpassantSeeds[i] = (unsigned long long)dist(rng);
    }

    turnSeed = (unsigned long long)dist(rng);
}

ZobristHasher::~ZobristHasher()
{
}

unsigned long long ZobristHasher::HashForPosition(bool isWhiteTurn, const Board& board, const TurnState& turnState, const unsigned int* whitePlayerPositions, const unsigned int* blackPlayerPositions)
{
    unsigned long long hash = 0;

    for (unsigned int i = 0; i < 16; i++)
    {
        int pos = whitePlayerPositions[i];
        bool isWhite = IsWhite(board[pos]);

        if (isWhite && IsValid(board[pos]) && GetId(board[pos]) == i)
        {
            hash = hash ^ pieceSeeds[(unsigned int)GetType(board[pos])  * 64 + pos];
        }

        pos = blackPlayerPositions[i];
        isWhite = IsWhite(board[pos]);

        if (!isWhite && IsValid(board[pos]) && GetId(board[pos]) == i)
        {
            hash = hash ^ pieceSeeds[((unsigned int)GetType(board[pos]) + 6) * 64 + pos];
        }
    }

    for (unsigned int i = 0; i < 4; i++)
    {
        if (!turnState.GetCastlingLegal(i & 1u, i & 2u))
        {
            hash = hash ^ castleSeeds[i];
        }
    }

    if (!isWhiteTurn)
    {
        hash = hash ^ turnSeed;
    }

    return hash;
}

unsigned long long ZobristHasher::HashForPiece(bool isWhite, PieceType pieceType, unsigned int position)
{
    return pieceSeeds[((isWhite ? 0u : 6u) + (unsigned int)pieceType) * 64 + position];
}

unsigned long long ZobristHasher::HashForEnpassant(unsigned int row)
{
    return enpassantSeeds[row];
}

unsigned long long ZobristHasher::HashForIsWhiteTurn()
{
    return turnSeed;
}

unsigned long long ZobristHasher::HashForCastling(bool white, bool right)
{
    return castleSeeds[((int)white * 2) + right];
}
