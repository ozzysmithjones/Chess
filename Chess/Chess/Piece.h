#pragma once
const int pieceValues[6]{ 100, 330,340, 500, 900, 10000 };

enum PieceType : unsigned int
{
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6
};

static inline int GetMaterial(PieceType type) { return pieceValues[(unsigned int)type - 1]; }

struct Piece
{
	union
	{
		struct
		{
			unsigned int id : 4;
			unsigned int isWhite : 1;
			unsigned int type : 3;
		};

		unsigned char data;
	};

	operator unsigned char& () { return data; }

	Piece();
	Piece(unsigned char data);
	Piece(unsigned int id, PieceType type, bool white);

	bool Valid() const { return data != 0; }
};

