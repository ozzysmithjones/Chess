#pragma once
#include <vector>
#include <memory>
#include "Move.h"

using namespace std;

struct Piece;
struct Board;
struct Move;
class GameState;

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

	bool IsWhite() { return isWhite; }
	int MiniMax(int depth, bool white, int alpha, int beta);

	virtual bool PrioritiseMoveA(const Move& a, const Move& b);
	virtual int EvaluatePosition(bool white, const Board& board, const std::vector<Move>& moves);


private:

	int			depth;
	bool		random = false;
	bool		isWhite;
	bool		isAI;
	Board*		board;
	GameState* gameState;
};

