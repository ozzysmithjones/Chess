#pragma once
#include <vector>
#include <memory>
#include "Chess.h"


int RowDiff(int position, int row);
int CenterDiff(int position, bool maximise = true);
int PosDiff(int position, int other);



class ChessPlayer
{
public:
	static void		setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, Chess* chess);
	ChessPlayer(Chess* chess, bool isWhite);

	void			SetAI(bool random, int depth);
	bool			IsAI() { return isAI; }
	bool			chooseAIMove(Move& moveToMake);

protected:

	bool IsWhitePlayer() { return isWhite; }

private:

	int			depth;
	bool		random = false;
	bool		isWhite;
	bool		isAI;
	Chess*		chess;
};

