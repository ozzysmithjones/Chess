#pragma once
#include <stdint.h>
#include "Piece.h"

struct TurnState
{
	//uint64_t zob;
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
