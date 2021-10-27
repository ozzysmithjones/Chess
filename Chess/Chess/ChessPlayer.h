#pragma once
#include <vector>
#include <memory>
#include "GameState.h"
#include "Move.h"

using namespace std;

struct PieceInPostion
{
	Piece piece;
	int col;
	int row;
};

typedef vector<PieceInPostion> vecPieces;

int CenterDiff(int position, bool maximise = true);
int PosDiff(int position, int other);

class ChessPlayer
{
public:
	static void		setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, GameState* gameState);
	ChessPlayer(GameState* gameState, bool isWhite);

	void			SetAI(bool random, int depth);
	bool			IsAI() { return isAI; }
	unsigned int	getAllLivePieces(vecPieces& vpieces);
	vector<Move>	getValidMovesForPiece(PieceInPostion pip);
	bool			chooseAIMove(Move& moveToMake);

protected:

	bool IsWhitePlayer() { return isWhite; }
	int MiniMax(int depth, bool white, int alpha, int beta);

	virtual bool PrioritiseMoveA(const Move& a, const Move& b) const;
	virtual int EvaluatePosition(bool white, const Board& board, const std::vector<Move>& moves);


private:

	int			depth;
	bool		random = false;
	bool		isWhite;
	bool		isAI;
	Board*		board;
	GameState* gameState;
};

