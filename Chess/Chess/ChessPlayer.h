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
	bool			chooseAIMove(std::vector<Move>& moves, Move& moveToMake);

protected:

	bool IsWhitePlayer() { return isWhite; }

	void PrioritiseMoves(std::vector<Move>& moves);
	int NegaMax(int depth, int alpha, int beta);
	int EvaluatePosition();

private:



	int			depth;
	bool		random = false;
	bool		isWhite;
	bool		isAI;
	Chess*		chess;
};

