#include "Piece.h"

Piece::Piece()
{
	data = 0;
}

Piece::Piece(unsigned short _data)
{
	data = _data;
}

Piece::Piece(PieceType _type, bool _white)
{
	type = _type;
	isWhite = _white;
}
