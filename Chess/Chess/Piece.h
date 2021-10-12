#pragma once
const unsigned int pieceValues[6]{ 100, 330,340, 500, 900, 10000 };

enum class PieceType : unsigned short
{
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6
};

static inline unsigned int GetMaterial(PieceType type) { return pieceValues[(unsigned int)type - 1]; }

struct Piece
{
	union
	{
		struct
		{
			PieceType type : 3;
			bool isWhite : 1;
		};

		unsigned short data;
	};

	operator unsigned short& () { return data; }

	Piece();
	Piece(unsigned short data);
	Piece(PieceType type, bool white);

	bool Valid() const { return data != 0; }
};

