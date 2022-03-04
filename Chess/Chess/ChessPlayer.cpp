#include <limits>
#include <algorithm>
#include "ChessPlayer.h"


void ChessPlayer::setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, Chess* chess)
{
	*playerBlack = new ChessPlayer(chess, false);
	(*playerBlack)->SetAI(true, 3);

	*playerWhite = new ChessPlayer(chess, true);
	//(*playerWhite)->SetAI(true, 1);
}

ChessPlayer::ChessPlayer(Chess* chess, bool isWhite)
	: chess(chess), isWhite(isWhite)
{

}

void ChessPlayer::SetAI(bool _random, int _depth)
{
	isAI = true;
	random = _random;
	depth = _depth;
}



// chooseAIMove
// in this method - for an AI chess player - choose a move to make. This is called once per play. 
bool ChessPlayer::chooseAIMove(Move& moveToMake)
{
	std::vector<Move> moves;
	chess->CalculateLegalMoves(moves);

	if (moves.empty())
	{
		return false;
	}

	if (random)
	{
		moveToMake = moves[rand() % moves.size()];
		return true;
	}

	//std::sort(moves.begin(), moves.end(), [this](const Move& a, const Move& b) { return this->PrioritiseMoveA(a, b); });
	moveToMake = moves[0];
	int bestScore = isWhite ? INT_MIN : INT_MAX;

	for (auto& move : moves)
	{
		//gameState->MakeMove(move);
		//int score = MiniMax(depth, gameState->IsWhiteTurn(), INT_MIN, INT_MAX);
		//gameState->UnmakeMove();

		//if ((score >= bestScore && isWhite) || (score <= bestScore && !isWhite))
		//{
		//	bestScore = score;
			//moveToMake = move;
		//}
	}

	return true;
}





int RowDiff(int position, int row)
{
	int r = (position >> 3);
	return abs(row - r);
}

int CenterDiff(int position, bool maximise)
{
	unsigned int x = position & 7;
	unsigned int y = position >> 3;

	const unsigned int centerMinX = 3;
	const unsigned int centerMinY = 3;
	const unsigned int centerMaxX = 4;
	const unsigned int centerMaxY = 4;

	int xDiff = std::min(abs((int)(centerMinX - x)), abs((int)(centerMaxX - x)));
	int yDiff = std::min(abs((int)(centerMinY - y)), abs((int)(centerMaxY - y)));

	return maximise ? std::max(xDiff, yDiff) : std::min(xDiff, yDiff);//(xDiff * xDiff) + (yDiff * yDiff); ///std::max(xDiff, yDiff);
}

int PosDiff(int position, int other)
{
	int xDiff = abs((int)((other & 7) - (position & 7)));
	int yDiff = abs((int)((other >> 3) - (position >> 3)));
	return std::max(xDiff, yDiff); //(xDiff * xDiff) + (yDiff * yDiff); ///std::max(xDiff, yDiff);
}
