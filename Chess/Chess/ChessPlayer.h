#pragma once
#include <unordered_map>
#include <vector>
#include "Chess.h"
#include "TranspositionTable.h"

constexpr auto MAX_PLY = 20;

int RowDiff(int position, int row);
int CenterDiff(int position, bool maximise = true);
int PosDiff(int position, int other);

class ChessPlayer
{
public:
	static void		setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, Chess& chess);
	ChessPlayer(Chess& chess, bool isWhite);
	~ChessPlayer();

	void			SetAI(bool random, int depth);
	bool			IsAI() { return isAI; }
	bool			chooseAIMove(std::vector<Move>& moves, Move& moveToMake);

protected:

	bool IsWhitePlayer() { return isWhite; }
	

	int ScoreMove(const Move move);
	void Sort(std::vector<Move>& moves, Move transpositionMove);
	int Evaluate(const GameState& state, bool isWhite);
	bool IsMovePromising(const Move move);
	int32_t Quiescence(int alpha, int beta);
	int32_t NegaMax(int alpha, int beta, int depth);

private:

	int			depth;
	bool		random = false;
	const bool	isWhite;
	bool		isAI;
	Chess&		chess;

	int ply = 0;
	Move killerMoves[2][MAX_PLY]{ 0 }; //[Index][Ply]
	unsigned historyMoveScores[6][64]{ 0 }; //[Piece][Move to square]
	TranspositionTable* transpositionTable;

	Move moveToPlay;
};

