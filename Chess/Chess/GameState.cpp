#include "GameState.h"
void GameState::MakeMove(const Move& move)
{
    log.push(move);
    board[move.startPosition] = 0;
    board[move.endPosition] = move.endPiece;

    //for castle:
    int left = move.endPiece.isWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
    int right = left + 7;

    //for enpassant
    enpassantableLog.push(NO_ENPASSANT);

    //for capture:
    switch (move.moveType)
    {
    case MoveType::NORMAL:

        if (move.startPiece.type == PieceType::PAWN && abs((int)((move.endPosition >> 3) - (move.startPosition >> 3))) == 2)
        {
            enpassantableLog.top() = move.endPosition;
        }

        break;
    case MoveType::ENPASSANT_LOWER:
        board[move.startPosition - 1] = 0;
        break;
    case MoveType::ENPASSANT_HIGHER:
        board[move.startPosition + 1] = 0;
        break;
    case MoveType::CASTLE_LOWER:
        board[move.endPosition + 1] = board[left];
        board[left] = 0;
        break;
    case MoveType::CASTLE_HIGHER:
        board[move.endPosition - 1] = board[right];
        board[right] = 0;
        break;
    }

    //shift turn
    isWhiteTurn = !isWhiteTurn;
}

void GameState::UnmakeMove()
{
    const Move& move = log.top();
    board[move.endPosition] = 0;
    board[move.startPosition] = move.startPiece;

    //for castle:
    int left = move.endPiece.isWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
    int right = left + 7;

    //for capture:

    switch (move.moveType)
    {
    case MoveType::NORMAL:
        board[move.endPosition] = move.capturedPiece;
        break;
    case MoveType::ENPASSANT_LOWER:
        board[move.startPosition - 1] = move.capturedPiece;
        break;
    case MoveType::ENPASSANT_HIGHER:
        board[move.startPosition + 1] = move.capturedPiece;
        break;
    case MoveType::CASTLE_LOWER:
        board[left] = board[move.endPosition + 1];
        board[move.endPosition + 1] = 0;
        break;
    case MoveType::CASTLE_HIGHER:
        board[right] = board[move.endPosition - 1];
        board[move.endPosition - 1] = 0;
        break;
    }

    //shift turn
    isWhiteTurn = !isWhiteTurn;
    enpassantableLog.pop();
    log.pop();
}

bool GameState::IsInCheck()
{
    return IsInCheck(false);
}

void GameState::GetAvalibleMoves(std::vector<Move>& moves, unsigned int position, Piece piece)
{
    switch (piece.type)
    {
    case PieceType::PAWN:
        GetPawnMoves(moves, position, piece);
        break;

    case PieceType::KNIGHT:
        GetKnightMoves(moves, position, piece);
        break;

    case PieceType::BISHOP:
        GetSlidingMoves(moves, position, piece, true);
        break;

    case PieceType::ROOK:
        GetSlidingMoves(moves, position, piece, false);
        break;

    case PieceType::QUEEN:
        GetSlidingMoves(moves, position, piece, false);
        GetSlidingMoves(moves, position, piece, true);
        break;

    case PieceType::KING:
        GetKingMoves(moves, position, piece);
        break;
    }
}

void GameState::GetAvalibleMoves(std::vector<Move>& moves)
{
    moves.reserve(20);

    for (unsigned int y = 0; y < Board::HEIGHT; y++)
    {
        for (unsigned int x = 0; x < Board::WIDTH; x++)
        {
            if (board.IsPieceAt(x, y) && board.At(x, y).isWhite == isWhiteTurn)
            {
                Piece piece = board.At(x, y);

                switch (piece.type)
                {
                case PieceType::PAWN:
                    GetPawnMoves(moves, (y << 3) + x, piece);
                    break;

                case PieceType::KNIGHT:
                    GetKnightMoves(moves, (y << 3) + x, piece);
                    break;

                case PieceType::BISHOP:
                    GetSlidingMoves(moves, (y << 3) + x, piece, true);
                    break;

                case PieceType::ROOK:
                    GetSlidingMoves(moves, (y << 3) + x, piece, false);
                    break;

                case PieceType::QUEEN:
                    GetSlidingMoves(moves, (y << 3) + x, piece, false);
                    GetSlidingMoves(moves, (y << 3) + x, piece, true);
                    break;

                case PieceType::KING:
                    GetKingMoves(moves, (y << 3) + x, piece);
                    break;
                }
            }
        }
    }
}



bool GameState::IsInCheck(bool opponent)
{
    Piece king = 0;

    for (unsigned int i = 0; i < 64u; i++)
    {
        if (board[i].type == PieceType::KING && ((board[i].isWhite == isWhiteTurn) == opponent))
        {
            king = board[i];
            break;
        }
    }

    if (king == 0)
        return false;

    for (unsigned int i = 0; i < 64u; i++)
    {
        if (board[i] != 0 && ((board[i].isWhite == isWhiteTurn) == opponent))
        {
            std::vector<Move> moves;
            GetAvalibleMoves(moves, i, board[i]);

            for (auto& move : moves)
            {
                if (move.capturedPiece.data == king.data)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

std::vector<Move> GameState::GetLegalMoves()
{
    std::vector<Move> moves;
    moves.reserve(20);
    GetAvalibleMoves(moves);

    for (unsigned int i = 0; i < moves.size();i++)
    {
        MakeMove(moves[i]);

        if (IsInCheck(true))
        {
            moves[i] = moves[moves.size() - 1];
            moves.pop_back();
            i--;
        }

        UnmakeMove();
    }

    return moves;
}

const std::vector<Move> GameState::GetLegalMoves(unsigned int x, unsigned int y)
{
    std::vector<Move> moves;
    GetAvalibleMoves(moves, (y << 3) + x, board[(y << 3) + x]);
    for (unsigned int i = 0; i < moves.size(); i++)
    {
        MakeMove(moves[i]);

        if (IsInCheck(true))
        {
            moves[i] = moves[moves.size() - 1];
            moves.pop_back();
            i--;
        }

        UnmakeMove();
    }

    return moves;
}

void GameState::GetPawnMoves(std::vector<Move>& moves, unsigned int position, Piece pawn)
{
    int direction = pawn.isWhite ? 8 : -8;
    unsigned int forward = position + direction;

    Piece endPiece = pawn;
    endPiece.moved = true;

    bool rightInBounds = board.InBounds((forward & 7) + 1, forward >> 3);
    bool leftInBounds = board.InBounds((forward & 7) - 1, forward >> 3);

    if (rightInBounds)
    {
        //Normal diagonal capture.

        if (board[forward + 1] != 0 && board[forward + 1].isWhite != pawn.isWhite)
        {
            AddPawnMove(moves, Move(MoveType::NORMAL, position, forward + 1, pawn, endPiece, board[forward + 1]));
        }

        //Enpassant

        if (board[forward + 1] == 0 && !enpassantableLog.empty() && (position + 1) == enpassantableLog.top())
        {
            AddPawnMove(moves, Move(MoveType::ENPASSANT_HIGHER, position, forward + 1, pawn, endPiece, board[position + 1]));
        }
    }

    if (leftInBounds)
    {
        //Normal diagonal capture.

        if (board[forward - 1] != 0 && board[forward - 1].isWhite != pawn.isWhite)
        {
            AddPawnMove(moves, Move(MoveType::NORMAL, position, forward - 1, pawn, endPiece, board[forward - 1]));
        }

        //Enpassant

        //board[forward - 1] == 0 && board[position - 1] != 0 && board[position + 1].type == PieceType::PAWN && board[position - 1].isWhite != pawn.isWhite
        if (board[forward + 1] == 0 && !enpassantableLog.empty() && (position - 1) == enpassantableLog.top())
        {
            AddPawnMove(moves, Move(MoveType::ENPASSANT_LOWER, position, forward - 1, pawn, endPiece, board[position - 1]));
        }
    }

    //Forward moves

    if (board[forward] == 0)
    {

        //Normal move

        AddPawnMove(moves, Move(MoveType::NORMAL, position, forward, pawn, endPiece));

        //Advanced pawn

        if (!pawn.moved && board[forward + direction] == 0)
        {
            AddPawnMove(moves, Move(MoveType::NORMAL, position, forward + direction, pawn, endPiece));
        }
    }
}

void GameState::GetSlidingMoves(std::vector<Move>& moves, unsigned int position, Piece piece, bool diagonal)
{
    Piece endPiece = piece;
    endPiece.moved = true;

    unsigned int xCenter = position & 7;
    unsigned int yCenter = position >> 3;

    int xDirections[4];
    int yDirections[4];

    if (diagonal)
    {
        xDirections[0] = -1;
        xDirections[1] = 1;
        xDirections[2] = -1;
        xDirections[3] = 1;

        yDirections[0] = 1;
        yDirections[1] = 1;
        yDirections[2] = -1;
        yDirections[3] = -1;
    }
    else
    {
        xDirections[0] = 1;
        xDirections[1] = 0;
        xDirections[2] = -1;
        xDirections[3] = 0;

        yDirections[0] = 0;
        yDirections[1] = -1;
        yDirections[2] = 0;
        yDirections[3] = 1;
    }

    for (unsigned int i = 0; i < 4u; i++)
    {
        int x = xCenter + xDirections[i];
        int y = yCenter + yDirections[i];

        while (board.InBounds(x, y))
        {
            unsigned int pos = (y << 3) + x;

            if (board[pos] != 0)
            {
                if (board[pos].isWhite != piece.isWhite)
                {
                    moves.emplace_back(MoveType::NORMAL, position, pos, piece, endPiece, board[pos]);
                }

                break;
            }

            moves.emplace_back(MoveType::NORMAL, position, pos, piece, endPiece);

            x += xDirections[i];
            y += yDirections[i];
        }
    }
}

void GameState::GetKnightMoves(std::vector<Move>& moves, unsigned int position, Piece knight)
{
    Piece endPiece = knight;
    endPiece.moved = true;

    unsigned int centerX = position & 7;
    unsigned int centerY = position >> 3;
    unsigned int x;
    unsigned int y;

    for (unsigned int i = 0; i < 8; i++)
    {
        x = centerX + ((i & 1u) != 0u ? 2 : 1) * ((i & 2u) != 0u ? 1 : -1);
        y = centerY + ((i & 1u) == 0u ? 2 : 1) * ((i & 4u) != 0u ? 1 : -1);

        if (board.InBounds(x, y) && (!board.IsPieceAt(x, y) || (board.At(x, y).isWhite != knight.isWhite)))
        {
            moves.emplace_back(MoveType::NORMAL, position, (y << 3) + x, knight, endPiece, board[(y << 3) + x]);
        }
    }
}

void GameState::GetKingMoves(std::vector<Move>& moves, unsigned int position, Piece king)
{
    Piece endPiece = king;
    endPiece.moved = true;

    unsigned int centerX = position & 7;
    unsigned int centerY = position >> 3;

    //1 step in any direction

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            unsigned int pos = ((centerY + y) << 3) + centerX + x;

            if (board.InBounds(centerX + x, centerY + y) && !(x == 0 && y == 0) && (board[pos] == 0 || board[pos].isWhite != king.isWhite))
            {
                moves.emplace_back(MoveType::NORMAL, position, pos, king, endPiece, board[pos]);
            }
        }
    }

    // castle TODO: prevent castling through check

    if (!king.moved)
    {
        int left = king.isWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
        int right = left + 7;

        if (board[left] != 0 && !board[left].moved && board[left].type == PieceType::ROOK && board[left].isWhite == king.isWhite && board[position - 1] == 0 && board[position - 2] == 0)
        {
            moves.emplace_back(MoveType::CASTLE_LOWER, position, position - 2, king, endPiece);
        }

        if (board[right] != 0 && !board[right].moved && board[right].type == PieceType::ROOK && board[right].isWhite == king.isWhite && board[position + 1] == 0 && board[position + 2] == 0)
        {
            moves.emplace_back(MoveType::CASTLE_HIGHER, position, position + 2, king, endPiece);
        }
    }
}

void GameState::AddPawnMove(std::vector<Move>& moves, const Move& move)
{
    bool whitePromote = move.startPiece.isWhite && (move.endPosition >> 3) == Board::MAX_ROW_INDEX - 1;
    bool blackPromote = !move.startPiece.isWhite && (move.endPosition >> 3) == Board::MIN_ROW_INDEX;
    if (whitePromote || blackPromote)
    {
        moves.emplace_back(move.moveType, move.startPosition, move.endPosition, move.startPiece, Piece(PieceType::QUEEN, true, move.startPiece.isWhite),move.capturedPiece);
        moves.emplace_back(move.moveType, move.startPosition, move.endPosition, move.startPiece, Piece(PieceType::ROOK, true, move.startPiece.isWhite),move.capturedPiece);
        moves.emplace_back(move.moveType, move.startPosition, move.endPosition, move.startPiece, Piece(PieceType::BISHOP, true, move.startPiece.isWhite),move.capturedPiece);
        moves.emplace_back(move.moveType, move.startPosition, move.endPosition, move.startPiece, Piece(PieceType::KNIGHT, true, move.startPiece.isWhite),move.capturedPiece);
    }
    else 
    {
        moves.push_back(move);
    }
}
