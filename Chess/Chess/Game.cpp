#include "Game.h"

Game::Game()
{
    gameState = new GameState();
    SetUpPlayerPieces(true);
    SetUpPlayerPieces(false);

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
    for (auto& m : moves)
    {
        if (m.endPosition == ((positionEndY << 3) + positionEndX)) 
        {
            move = m;
            return true;
        }
    }

    return false;
}

void Game::SetUpPlayerPieces(bool white)
{
    Board& board = gameState->GetBoard();
    unsigned int firstRow = white ? Board::MIN_ROW_INDEX : Board::MAX_ROW_INDEX - 1;
    unsigned int secondRow = white ? Board::MIN_ROW_INDEX + 1 : Board::MAX_ROW_INDEX - 2;

    //Order of the major pieces as they are on a chess board
    PieceType pieces[] = { PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
        PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK };

    //Place pieces down:
    for (unsigned int x = Board::MIN_COL_INDEX; x < Board::MAX_ROW_INDEX; x++)
    {
        board.At(x, firstRow) = Piece(pieces[x], false, white);
        board.At(x, secondRow) = Piece(PieceType::PAWN, false, white);
    }
}
