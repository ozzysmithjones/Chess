#include "TurnState.h"

void TurnState::SetCastlingIllegal(bool white)
{
    preventedCastling = true;

    if (white)
    {
        castlingLegality[0] = false;
        castlingLegality[1] = false;
    }
    else
    {
        castlingLegality[2] = false;
        castlingLegality[3] = false;
    }
}

void TurnState::SetCastlingIllegal(bool white, bool right)
{
    preventedCastling = true;

    if (white)
    {
        castlingLegality[right] = false;
    }
    else
    {
        castlingLegality[2 + right] = false;
    }
}

bool TurnState::GetCastlingLegal(bool white, bool right) const
{
    return castlingLegality[(white ? 0 : 2) + right];
}

bool TurnState::GetCastlingLegal(bool white) const
{
    if (white)
    {
        return castlingLegality[0] || castlingLegality[1];
    }
    else
    {
        return castlingLegality[2] || castlingLegality[3];
    }
}

TurnState::TurnState()
{
    enpassantPosition = 255;
    capturedPiece = 0;
    castlingLegality[0] = true;
    castlingLegality[1] = true;
    castlingLegality[2] = true;
    castlingLegality[3] = true;
}