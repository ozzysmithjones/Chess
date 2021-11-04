#pragma once
#include <stack>
#include <vector>
#include <unordered_map>
#include "ZobristHasher.h"
#include "Board.h"
#include "TurnState.h"
#include "Move.h"

const unsigned int NO_ENPASSANT = 255u;

class GameState;

struct Player
{
	unsigned int positions[16]{ 0 };
	unsigned int pressure[64]{ 0 };
};

class GameState
{
public:

	GameState();
    ~GameState();

	void SetUpPlayerPieces(bool white);
    inline const std::stack<TurnState>& GetStateLog() const { return stateLog; }
	inline const Board& GetBoard() const { return board; }
	inline Board& GetBoardRef() { return board; }
	inline unsigned int* GetWhitePositions() { return whitePlayer.positions; }
	inline unsigned int* GetBlackPositions() { return blackPlayer.positions; }
	void MakeMove(const Move& move);
	void UnmakeMove();
    inline unsigned long long GetPositionZobristKey() const { return zobristKey; }

	bool IsInCheck();
    bool IsInCheck(bool white);
	inline bool IsWhiteTurn() const { return isWhiteTurn; }
	std::vector<Move> GetLegalMoves();
	const std::vector<Move> GetLegalMoves(unsigned int x, unsigned int y);

private:

	void GetAvalibleMoves(std::vector<Move>& moves, unsigned int position, Piece piece);
	void GetAvalibleMoves(std::vector<Move>& moves);

	bool isWhiteTurn = true;
	std::stack<Move> moveLog;
	std::stack<TurnState> stateLog;
	Board board;

	const unsigned int kingId = 4;
	Player whitePlayer;
	Player blackPlayer;

    ZobristHasher* zobristHasher;
    unsigned long long zobristKey;
    
	void AddPawnMoves(bool whitePiece, unsigned int position, const std::stack<TurnState>& stateLog, const Board& board, std::vector<Move>& moves);
	void AddKnightMoves(bool isWhite, unsigned int position, const Board& board, std::vector<Move>& moves);
	void AddBishopMoves(bool isWhite, unsigned int position, const Board& board, std::vector<Move>& moves);
	void AddRookMoves(bool isWhite, unsigned int position, const Board& board, std::vector<Move>& moves);
	void AddKingMoves(bool isWhite, unsigned int position, const std::stack<TurnState>& stateLog, const Board& board, std::vector<Move>& moves);
    

	static void AddPawnMove(std::vector<Move>& moves, Move pawnMove, bool isWhite);
};

const int diagX[4]{ 1, -1, 1, -1 };
const int diagY[4]{ 1,  1,-1, -1 };
const int orthoX[4]{ 0, 1,  0, -1 };
const int orthoY[4]{ 1,  0, -1, 0 };

#pragma region Move Function templates 

template<typename T>
void OnPawnMoves(bool whitePiece, unsigned int position, const std::stack<TurnState>& stateLog, const Board& board, const T& moveFunc)
{
    const unsigned int y = (position >> 3);
    const unsigned int x = (position & 7);
    unsigned int enPassantablePosition = stateLog.empty() ? NO_ENPASSANT : stateLog.top().enpassantPosition;
    unsigned int promoteRow;
    unsigned int advanceRow;
    unsigned int direction;

    if (whitePiece)
    {
        promoteRow = 7;
        advanceRow = 1;
        direction = 8;
    }
    else
    {
        promoteRow = 0;
        advanceRow = 6;
        direction = -8;
    }

    MoveType defaultType = (y + direction) != promoteRow ? MoveType::NORMAL : MoveType::PROMOTION;

    //forward move:

    if (board[position + direction] == 0)
    {
        moveFunc(position, position + direction, defaultType, PieceType::NONE);

        if (y == advanceRow && board[position + direction + direction] == 0)
        {
            moveFunc(position, position + direction + direction, MoveType::ADVANCED_PAWN, PieceType::NONE);
        }
    }

    //diagonal capture

    if (x != 7)
    {
        if (board[position + direction + 1] != 0 && IsWhite(board[position + direction + 1]) != whitePiece)
        {
            moveFunc(position, position + direction + 1, defaultType, GetType(board[position + direction + 1]));
        }

        if (position + 1 == enPassantablePosition && board[position + direction + 1] == 0 && IsWhite(board[position + 1]) != whitePiece)
        {
            moveFunc(position, position + direction + 1, MoveType::ENPASSANT_HIGHER, PieceType::PAWN);
        }
    }

    if (x != 0)
    {
        if (board[position + direction - 1] != 0 && IsWhite(board[position + direction - 1]) != whitePiece)
        {
            moveFunc(position, position + direction - 1, defaultType, GetType(board[position + direction + 1]));
        }

        if (position - 1 == enPassantablePosition && board[position + direction - 1] == 0 && IsWhite(board[position - 1]) != whitePiece)
        {
            moveFunc(position, position + direction - 1, MoveType::ENPASSANT_LOWER, PieceType::PAWN);
        }
    }
}



template<typename T>
void OnKnightMoves(bool isWhite, unsigned int position, const Board& board, const T& moveFunc)
{
    const unsigned int centerX = (position & 7);
    const unsigned int centerY = (position >> 3);
    unsigned int x;
    unsigned int y;
    unsigned int endPosition;

    for (unsigned int i = 0; i < 8u; i++)
    {
        x = centerX + ((i & 1u) != 0u ? 2 : 1) * ((i & 2u) != 0u ? 1 : -1);
        y = centerY + ((i & 1u) == 0u ? 2 : 1) * ((i & 4u) != 0u ? 1 : -1);
        endPosition = ToIndex(x, y);

        if (board.InBounds(x, y) && (board[endPosition] == 0 || IsWhite(board[endPosition]) != isWhite))
        {
            moveFunc(position, endPosition, MoveType::NORMAL, GetType(board[endPosition]));
        }
    }
}

template<typename T>
void OnBishopMoves(bool isWhite, unsigned int position, const Board& board, const T& moveFunc)
{
    const unsigned int centerY = (position >> 3);
    const unsigned int centerX = (position & 7);
    int x, y;
    int xDelta, yDelta;
    unsigned int endPosition;

    for (int i = 0; i < 4; i++)
    {
        yDelta = diagY[i];
        xDelta = diagX[i];
        x = centerX + xDelta;
        y = centerY + yDelta;
        endPosition = ToIndex(x, y);

        while (board.InBounds(x, y))
        {
            if (board[endPosition] != 0)
            {
                if (IsWhite(board[endPosition]) != isWhite)
                {
                    moveFunc(position, endPosition, MoveType::NORMAL, GetType(board[endPosition]));
                }

                break;
            }

            moveFunc(position, endPosition, MoveType::NORMAL, PieceType::NONE);

            x += xDelta;
            y += yDelta;
            endPosition = ToIndex(x, y);
        }
    }
}

template<typename T>
void OnRookMoves(bool isWhite, unsigned int position, const Board& board, const T& moveFunc)
{
    const unsigned int centerY = (position >> 3);
    const unsigned int centerX = (position & 7);
    int x, y;
    int xDelta, yDelta;
    unsigned int endPosition;

    for (int i = 0; i < 4; i++)
    {
        yDelta = orthoY[i];
        xDelta = orthoX[i];
        x = centerX + xDelta;
        y = centerY + yDelta;
        endPosition = ToIndex(x, y);

        while (board.InBounds(x, y))
        {
            if (board[endPosition] != 0)
            {
                if (IsWhite(board[endPosition]) != isWhite)
                {
                    moveFunc(position, endPosition, MoveType::NORMAL, GetType(board[endPosition]));
                }

                break;
            }

            moveFunc(position, endPosition, MoveType::NORMAL, PieceType::NONE);

            x += xDelta;
            y += yDelta;
            endPosition = ToIndex(x, y);
        }
    }
}

template<typename T>
void OnKingMoves(bool isWhite, unsigned int position, const std::stack<TurnState>& stateLog, const Board& board, const T& moveFunc)
{
    const unsigned int centerY = (position >> 3);
    const unsigned int centerX = (position & 7);

    //1 step any direction:

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            unsigned int endPosition = ToIndex(centerX + x, centerY + y);

            if (board.InBounds(centerX + x, centerY + y) && !(x == 0 && y == 0) && (board[endPosition] == 0 || IsWhite(board[endPosition]) != isWhite))
            {
                moveFunc(position, endPosition, MoveType::NORMAL, GetType(board[endPosition]));
            }
        }
    }


    //castle:

    bool leftLegal = true;
    bool rightLegal = true;

    if (!stateLog.empty())
    {
        rightLegal = stateLog.top().GetCastlingLegal(isWhite, true);
        leftLegal = stateLog.top().GetCastlingLegal(isWhite, false);
    }

    unsigned int left = isWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
    unsigned int right = left + 7;

    if (leftLegal && board[left] != 0 && GetType(board[left]) == PieceType::ROOK && IsWhite(board[left]) == isWhite && board[position - 1] == 0 && board[position - 2] == 0)
    {
        moveFunc(position, position - 2, MoveType::CASTLE_LOWER, PieceType::NONE);
    }

    if (rightLegal && board[right] != 0 && GetType(board[right]) == PieceType::ROOK && IsWhite(board[right]) == isWhite && board[position + 1] == 0 && board[position + 2] == 0)
    {
        moveFunc(position, position + 2, MoveType::CASTLE_HIGHER, PieceType::NONE);
    }
}

template<typename T>
void OnPieceMoves(bool whitePiece, PieceType type, unsigned int position, const std::stack<TurnState>& stateLog, const Board& board, const T& moveFunc)
{

    switch (type)
    {
    case PieceType::PAWN:
        OnPawnMoves<T>(whitePiece, position, stateLog, board, moveFunc);
        break;
    case PieceType::KNIGHT:
        OnKnightMoves<T>(whitePiece, position, board, moveFunc);
        break;
    case PieceType::BISHOP:
        OnBishopMoves<T>(whitePiece, position, board, moveFunc);
        break;
    case PieceType::ROOK:
        OnRookMoves<T>(whitePiece, position, board, moveFunc);
        break;
    case PieceType::QUEEN:
        OnBishopMoves<T>(whitePiece, position, board, moveFunc);
        OnRookMoves<T>(whitePiece, position, board, moveFunc);
        break;
    case PieceType::KING:
        OnKingMoves<T>(whitePiece, position, stateLog, board, moveFunc);
        break;
    }

}

#pragma endregion

