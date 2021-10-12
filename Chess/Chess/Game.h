#pragma once
#include "ChessPlayer.h"
#include "GameState.h"

class Game
{
public:

	Game();
	~Game();

	//For drawing and UI
	Board& GetChessBoard() { return gameState->GetBoard(); }
	bool IsWhiteTurn() const { return gameState->IsWhiteTurn(); }
	bool IsCheck() { return gameState->IsInCheck(); }
	bool IsCheckMate() { return gameState->GetLegalMoves().empty(); }
	std::vector<Move> GetLegalmoves(unsigned int x, unsigned int y) { return gameState->GetLegalMoves(x,y); }

	//For user Input:
	void MakePlayerMove(Move move);
	void Undo();
	bool InterpretMove(Move& move, unsigned int positionStartX, unsigned int positionStartY, unsigned int positionEndX, unsigned int positionEndY);

	//Getting players.
	inline ChessPlayer* GetWhitePlayer() { return whitePlayer; }
	inline ChessPlayer* GetBlackPlayer() { return blackPlayer; }

private:

	void SetUpPlayerPieces(bool white);
	GameState* gameState;
	ChessPlayer* whitePlayer;
	ChessPlayer* blackPlayer;
};

