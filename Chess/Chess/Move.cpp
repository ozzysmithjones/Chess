#include <iostream>
#include "Move.h"

void PrintMove(const Move& move)
{
	if (move == 0)
	{
		std::wcout << L"[0]";
	}
	else
	{
		std::wcout << L"[" << squareNames[GetStartPosition(move)] << squareNames[GetEndPosition(move)] << L"]";
	}
}

Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType)
{
	return position | (endPosition << 6) | ((unsigned int)moveType << 12) | ((unsigned int)pieceType << 15);
}

Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType, PieceType captureType)
{
	return position | (endPosition << 6) | ((unsigned int)moveType << 12) | ((unsigned int)pieceType << 15) | ((unsigned int)captureType << 18);
}

Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType, PieceType captureType, PieceType promoteType)
{
	return position | (endPosition << 6) | ((unsigned int)moveType << 12) | ((unsigned int)pieceType << 15) | ((unsigned int)captureType << 18) | ((unsigned int)promoteType << 21);
}

Move CreateMove(unsigned int position, unsigned int endPosition, MoveType moveType, PieceType pieceType, PieceType captureType, PieceType promoteType, bool isCheck, bool isPin)
{
	return position | (endPosition << 6) | ((unsigned int)moveType << 12) | ((unsigned int)pieceType << 15) | ((unsigned int)captureType << 18) | ((unsigned int)promoteType << 21)
		| ((unsigned int)isCheck << 22) | ((unsigned int)isPin << 23);
}
