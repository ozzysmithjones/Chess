
#include <limits>
#include <random>
#include "ZobristHasher.h"

typedef std::mt19937_64 RNG;
const uint32_t seed_val = 137;

ZobristHasher::ZobristHasher()
{
    RNG rng;
    rng.seed(seed_val);
    std::uniform_int_distribution<uint64_t> dist;

    for (unsigned int i = 0; i < 64 * 12; i++)
    {
        pieceSeeds[i] = dist(rng);//rand() % std::numeric_limits<unsigned long long>::max();
    }

    for (unsigned int i = 0; i < 4; i++)
    {
        castleSeeds[i] = dist(rng);
    }

    for (unsigned int i = 0; i < 8; i++)
    {
        enpassantSeeds[i] = dist(rng);
    }

    turnSeed = dist(rng);
}

ZobristHasher::~ZobristHasher()
{
}

uint64_t ZobristHasher::HashForPosition(bool isWhiteTurn, const Board& board, const TurnState& turnState, const unsigned int* whitePlayerPositions, const unsigned int* blackPlayerPositions)
{
    uint64_t hash = 0;

    for (unsigned int i = 0; i < 4; i++)
    {
        if (!turnState.GetCastlingLegal(i & 1u, i & 2u))
        {
            hash = hash ^ castleSeeds[i];
        }
    }

    if (turnState.enpassantPosition != 255)
    {
        hash = hash ^ HashForEnpassant(turnState.enpassantPosition >> 3);
    }

    if (!isWhiteTurn)
    {
        hash = hash ^ turnSeed;
    }

    for (unsigned int i = 0; i < 16; i++)
    {
        int pos = whitePlayerPositions[i];
        bool isWhite = IsWhite(board[pos]);

        if (isWhite && IsValid(board[pos]) && GetId(board[pos]) == i)
        {
            hash = hash ^ pieceSeeds[((unsigned int)GetType(board[pos])-1)  * 64 + pos];
        }

        pos = blackPlayerPositions[i];
        isWhite = IsWhite(board[pos]);

        if (!isWhite && IsValid(board[pos]) && GetId(board[pos]) == i)
        {
            hash = hash ^ pieceSeeds[((unsigned int)GetType(board[pos]) + 5) * 64 + pos];
        }
    }

    return hash;
}

uint64_t ZobristHasher::HashForPiece(bool isWhite, PieceType pieceType, unsigned int position)
{
    return pieceSeeds[((isWhite ? 0u : 6u) + ((unsigned int)pieceType-1)) * 64 + position];
}

uint64_t ZobristHasher::HashForEnpassant(unsigned int row)
{
    return enpassantSeeds[row];
}

uint64_t ZobristHasher::HashForIsWhiteTurn()
{
    return turnSeed;
}

uint64_t ZobristHasher::HashForCastling(bool white, bool right)
{
    return castleSeeds[((int)white * 2) + right];
}
