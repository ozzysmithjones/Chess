#include "Game.h"

Game::Game()
{
    gameState = new GameState();
    gameState->SetUpPlayerPieces(true);
    gameState->SetUpPlayerPieces(false);

    ChessPlayer::setupPlayers(&whitePlayer, &blackPlayer, gameState);
}

Game::~Game()
{
    delete gameState;
    gameState = nullptr;
}

void Game::MakePlayerMove(Move move)
{
    gameState->MakeMove(move);
}

void Game::Undo()
{
    gameState->UnmakeMove();
}

bool Game::InterpretMove(Move& move, unsigned int positionStartX, unsigned int positionStartY, unsigned int positionEndX, unsigned int positionEndY)
{
    std::vector<Move> moves = gameState->GetLegalMoves(positionStartX, positionStartY);
    for (const auto& m : moves)
    {
        if (GetEndPos(m) == ((positionEndY << 3) + positionEndX)) 
        {
            move = m;
            return true;
        }
    }

    return false;
}

