#include "Piece.h"

Piece::Piece()
{
	data = 0;
}

Piece::Piece(unsigned int _data)
{
	data = _data;
}

Piece::Piece(PieceType _type, bool _moved, bool _white)
{
	type = _type;
	moved = _moved;
	isWhite = _white;
}
