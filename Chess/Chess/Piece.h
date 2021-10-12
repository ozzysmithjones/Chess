#pragma once
const unsigned int pieceValues[6]{ 100, 330,340, 500, 900, 10000 };

enum class PieceType : unsigned int
{
	PAWN = 1,
	KNIGHT = 2,
	BISHOP = 3,
	ROOK = 4,
	QUEEN = 5,
	KING = 6
};

struct Piece
{
	union
	{
		struct
		{
			PieceType type : 4;
			bool isWhite : 1;
			bool moved : 1;
		};

		unsigned int data;
	};

	operator unsigned int& () { return data; }

	Piece();
	Piece(unsigned int data);
	Piece(PieceType type, bool moved, bool white);

	bool Valid() const { return data != 0; }
	static unsigned int GetMaterialValue(PieceType type) { return pieceValues[(unsigned int)type - 1]; }
};

