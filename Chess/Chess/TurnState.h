#pragma once
#include "Piece.h"

struct TurnState
{
	bool preventedCastling;
	unsigned int enpassantPosition;
	Piece capturedPiece;
	Piece capturedPieceEnpassant;

	void SetCastlingIllegal(bool white);
	void SetCastlingIllegal(bool white, bool right);
	bool GetCastlingLegal(bool white, bool right) const;
	bool GetCastlingLegal(bool white) const;

	TurnState();

private:

	bool castlingLegality[4];
};


