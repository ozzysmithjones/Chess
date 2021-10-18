#include "Piece.h"

Piece::Piece()
{
	data = 0;
}

Piece::Piece(unsigned char _data)
{
	data = _data;
}

Piece::Piece(unsigned int _id, PieceType _type, bool _white)
{
	id = _id;
	type = (unsigned int)_type;
	isWhite = _white;
}
