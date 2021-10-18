#pragma once
#include <stack>
#include <vector>
#include <unordered_map>
#include "Board.h"
#include "Move.h"

const unsigned int NO_ENPASSANT = 255u;


struct TurnState
{
	
	unsigned int enpassantPosition;
	Piece capturedPiece;
	Piece capturedPieceEnpassant;

	void SetCastlingIllegal(bool white);
	void SetCastlingIllegal(bool white, bool right);
	bool GetCastlingLegal(bool white, bool right);

	TurnState();

private:

	bool castlingLegality[4];
};

class GameState
{
public:

	GameState();

	void SetUpPlayerPieces(bool white);
	inline Board& GetBoard() { return board; }
	inline unsigned int* GetWhitePositions() { return whitePositions; }
	inline unsigned int* GetBlackPositions() { return blackPositions; }
	void MakeMove(const Move& move);
	void UnmakeMove();

	bool IsInCheck();
	inline bool IsWhiteTurn() const { return isWhiteTurn; }
	std::vector<Move> GetLegalMoves();
	const std::vector<Move> GetLegalMoves(unsigned int x, unsigned int y);

private:

	bool IsInCheck(bool white);
	
	void GetAvalibleMoves(std::vector<Move>& moves, unsigned int position, Piece piece);
	void GetAvalibleMoves(std::vector<Move>& moves);

	bool isWhiteTurn = true;
	std::stack<Move> moveLog;
	std::stack<TurnState> stateLog;
	Board board;

	const unsigned int kingId = 4;

	unsigned int whitePositions[16];
	unsigned int blackPositions[16];

	void GetPawnMoves(std::vector<Move>& moves, unsigned int position, Piece pawn);
	void GetSlidingMoves(std::vector<Move>& moves, unsigned int position, Piece piece, bool diagonal);
	void GetKnightMoves(std::vector<Move>& moves, unsigned int position, Piece knight);
	void GetKingMoves(std::vector<Move>& moves, unsigned int position, Piece king);
	void AddPawnMove(std::vector<Move>& moves, const Move& pawnMove, bool isWhite);
};



