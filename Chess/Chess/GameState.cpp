#include "GameState.h"
GameState::GameState()
{

}
void GameState::MakeMove(const Move& move)
{
    TurnState turnState = stateLog.empty() ? TurnState() : stateLog.top();
    turnState.enpassantPosition = NO_ENPASSANT;
    turnState.capturedPiece = board[move.endPosition];

    Piece piece = board[move.startPosition];
    board[move.endPosition] = piece;
    board[move.startPosition] = 0;

    unsigned int* positions = piece.isWhite ? whitePlayer.positions : blackPlayer.positions;
    positions[piece.id] = move.endPosition;

    //castle directions:
    int left = piece.isWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
    int right = left + 7;

    switch (move.moveType)
    {
    case MoveType::NORMAL:

        //advanced pawn
        if (piece.type == PieceType::PAWN && abs((int)((move.endPosition >> 3) - (move.startPosition >> 3))) == 2)
        {
            turnState.enpassantPosition = move.endPosition;
        }

        //castling made illegal

        if (piece.type == PieceType::KING)
        {
            turnState.SetCastlingIllegal(piece.isWhite);
        }

        if (piece.type == PieceType::ROOK)
        {
            if (move.startPosition == right)
            {
                turnState.SetCastlingIllegal(piece.isWhite, true);
            }
            else if (move.startPosition == left)
            {
                turnState.SetCastlingIllegal(piece.isWhite, false);
            }
        }

        break;

    case MoveType::ENPASSANT_LOWER:
        turnState.capturedPieceEnpassant = board[move.startPosition - 1];
        board[move.startPosition - 1] = 0;
        break;
    case MoveType::ENPASSANT_HIGHER:
        turnState.capturedPieceEnpassant = board[move.startPosition + 1];
        board[move.startPosition + 1] = 0;
        break;
    case MoveType::CASTLE_LOWER:

    {
        Piece rook = board[left];
        positions[rook.id] = move.endPosition + 1;
        board[move.endPosition + 1] = rook;
        board[left] = 0;
    }

        turnState.SetCastlingIllegal(piece.isWhite);
        break;
    case MoveType::CASTLE_HIGHER:

    {
        Piece rook = board[right];
        positions[rook.id] = move.endPosition - 1;
        board[move.endPosition - 1] = rook;
        board[right] = 0;
    }

        turnState.SetCastlingIllegal(piece.isWhite);
        break;

    case MoveType::PROMOTE_QUEEN:
        board[move.endPosition].type = PieceType::QUEEN;
        break;
    case MoveType::PROMOTE_ROOK:
        board[move.endPosition].type = PieceType::ROOK;
        break;
    case MoveType::PROMOTE_BISHOP:
        board[move.endPosition].type = PieceType::BISHOP;
        break;
    case MoveType::PROMOTE_KNIGHT:
        board[move.endPosition].type = PieceType::KNIGHT;
        break;
    }

    //shift turn
    moveLog.push(move);
    stateLog.push(turnState);
    isWhiteTurn = !isWhiteTurn;
}

void GameState::UnmakeMove()
{
    const Move& move = moveLog.top();
    moveLog.pop();

    Piece piece = board[move.endPosition];
    board[move.startPosition] = piece;
    board[move.endPosition] = stateLog.empty() ? Piece(0) : stateLog.top().capturedPiece;

    unsigned int* positions = piece.isWhite ? whitePlayer.positions : blackPlayer.positions;
    positions[piece.id] = move.startPosition;

    //for castle:
    int left = piece.isWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
    int right = left + 7;

    //for capture:
    switch (move.moveType)
    {
    case MoveType::NORMAL:
        break;
    case MoveType::ENPASSANT_LOWER:
        board[move.startPosition - 1] = stateLog.top().capturedPieceEnpassant;
        break;
    case MoveType::ENPASSANT_HIGHER:
        board[move.startPosition + 1] = stateLog.top().capturedPieceEnpassant;
        break;
    case MoveType::CASTLE_LOWER:

    {
        Piece rook = board[move.endPosition + 1];
        positions[rook.id] = left;
        board[left] = rook;
        board[move.endPosition + 1] = 0;
    }
        break;
    case MoveType::CASTLE_HIGHER:

    {
        Piece rook = board[move.endPosition - 1];
        positions[rook.id] = right;
        board[right] = rook;
        board[move.endPosition - 1] = 0;
    }
        break;
    case MoveType::PROMOTE_QUEEN:
    case MoveType::PROMOTE_ROOK:
    case MoveType::PROMOTE_BISHOP:
    case MoveType::PROMOTE_KNIGHT:
        board[move.startPosition].type = PieceType::PAWN;
        break;
    }


    //shift turn
    isWhiteTurn = !isWhiteTurn;
    stateLog.pop();
}

bool GameState::IsInCheck()
{
    return IsInCheck(isWhiteTurn);
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



bool GameState::IsInCheck(bool white)
{
    unsigned int kingPos = white ? whitePlayer.positions[kingId] : blackPlayer.positions[kingId];
    unsigned int* opponent = white ? blackPlayer.positions : whitePlayer.positions;

    for (unsigned int i = 0; i < 16;i++)
    {
        int pos = opponent[i];
        if (board[pos].Valid() && board[pos].id == i && board[pos].isWhite != white)
        {
            std::vector<Move> moves;
            GetAvalibleMoves(moves, pos, board[pos]);

            for (auto& move : moves)
            {
                if (move.endPosition == kingPos)
                {
                    return true;
                }
            }
        }
    }
    
    return false;
}

void GameState::SetUpPlayerPieces(bool white)
{
    unsigned int firstRow = white ? Board::MIN_ROW_INDEX : Board::MAX_ROW_INDEX - 1;
    unsigned int secondRow = white ? Board::MIN_ROW_INDEX + 1 : Board::MAX_ROW_INDEX - 2;
    unsigned int* positions = white ? whitePlayer.positions : blackPlayer.positions;

    //Order of the major pieces as they are on a chess board
    PieceType pieces[] = { PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
        PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK };

    //Place pieces down:
    for (unsigned int x = Board::MIN_COL_INDEX; x < Board::MAX_ROW_INDEX; x++)
    {
        board.At(x, firstRow) = Piece(x,pieces[x], white);
        board.At(x, secondRow) = Piece(x + 8,PieceType::PAWN, white);
        positions[x] = (firstRow << 3) + x;
        positions[x + 8] = (secondRow << 3) + x;
    }
}

std::vector<Move> GameState::GetLegalMoves()
{
    std::vector<Move> moves;
    moves.reserve(20);
    GetAvalibleMoves(moves);

    
    for (unsigned int i = 0; i < moves.size();i++)
    {
        MakeMove(moves[i]);

        if (IsInCheck(!isWhiteTurn))
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

        if (IsInCheck(!isWhiteTurn))
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

    bool rightInBounds = board.InBounds((forward & 7) + 1, forward >> 3);
    bool leftInBounds = board.InBounds((forward & 7) - 1, forward >> 3);

    if (rightInBounds)
    {
        //Normal diagonal capture.

        if (board[forward + 1] != 0 && board[forward + 1].isWhite != pawn.isWhite)
        {
            AddPawnMove(moves, Move(MoveType::NORMAL,position, forward + 1),pawn.isWhite);
        }

        //Enpassant

        if (board[forward + 1] == 0 && !stateLog.empty() && (position + 1) == stateLog.top().enpassantPosition)
        {
            AddPawnMove(moves, Move(MoveType::ENPASSANT_HIGHER, position, forward + 1),pawn.isWhite);
        }
    }

    if (leftInBounds)
    {
        //Normal diagonal capture.

        if (board[forward - 1] != 0 && board[forward - 1].isWhite != pawn.isWhite)
        {
            AddPawnMove(moves, Move(MoveType::NORMAL, position, forward - 1),pawn.isWhite);
        }

        //Enpassant

        //board[forward - 1] == 0 && board[position - 1] != 0 && board[position + 1].type == PieceType::PAWN && board[position - 1].isWhite != pawn.isWhite
        if (board[forward + 1] == 0 && !stateLog.empty() && (position - 1) == stateLog.top().enpassantPosition)
        {
            AddPawnMove(moves, Move(MoveType::ENPASSANT_LOWER, position, forward - 1),pawn.isWhite);
        }
    }

    //Forward moves

    if (board[forward] == 0)
    {

        //Normal move

        AddPawnMove(moves, Move(MoveType::NORMAL, position, forward),pawn.isWhite);

        //Advanced pawn

        const unsigned int start = pawn.isWhite ? 1 : 6;

        if (((position >> 3) == start) && board[forward + direction] == 0)
        {
            AddPawnMove(moves, Move(MoveType::NORMAL, position, forward + direction), pawn.isWhite);
        }
    }
}

void GameState::GetSlidingMoves(std::vector<Move>& moves, unsigned int position, Piece piece, bool diagonal)
{
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
                    moves.emplace_back(MoveType::NORMAL, position, pos);
                }

                break;
            }

            moves.emplace_back(MoveType::NORMAL, position, pos);

            x += xDirections[i];
            y += yDirections[i];
        }
    }
}

void GameState::GetKnightMoves(std::vector<Move>& moves, unsigned int position, Piece knight)
{
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
            moves.emplace_back(MoveType::NORMAL, position, (y << 3) + x);
        }
    }
}

void GameState::GetKingMoves(std::vector<Move>& moves, unsigned int position, Piece king)
{
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
                moves.emplace_back(MoveType::NORMAL, position, pos);
            }
        }
    }

    // castle TODO: prevent castling through check

    int left = king.isWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
    int right = left + 7;
    bool rightLegal = !stateLog.empty() && stateLog.top().GetCastlingLegal(king.isWhite, true);
    bool leftLegal = !stateLog.empty() && stateLog.top().GetCastlingLegal(king.isWhite, false);

    if (leftLegal && board[left] != 0 && board[left].type == PieceType::ROOK && board[left].isWhite == king.isWhite && board[position - 1] == 0 && board[position - 2] == 0)
    {
        moves.emplace_back(MoveType::CASTLE_LOWER, position, position - 2);
    }

    if (rightLegal && board[right] != 0 && board[right].type == PieceType::ROOK && board[right].isWhite == king.isWhite && board[position + 1] == 0 && board[position + 2] == 0)
    {
        moves.emplace_back(MoveType::CASTLE_HIGHER, position, position + 2);
    }
}

void GameState::AddPawnMove(std::vector<Move>& moves, const Move& move, bool isWhite)
{
    bool whitePromote = isWhite && (move.endPosition >> 3) == Board::MAX_ROW_INDEX - 1;
    bool blackPromote = !isWhite && (move.endPosition >> 3) == Board::MIN_ROW_INDEX;

    if (whitePromote || blackPromote)
    {
        moves.emplace_back(MoveType::PROMOTE_QUEEN, move.startPosition, move.endPosition);
        moves.emplace_back(MoveType::PROMOTE_ROOK, move.startPosition, move.endPosition);
        moves.emplace_back(MoveType::PROMOTE_BISHOP, move.startPosition, move.endPosition);
        moves.emplace_back(MoveType::PROMOTE_KNIGHT, move.startPosition, move.endPosition);
    }
    else 
    {
        moves.push_back(move);
    }
}

void GameState::GetPins(std::vector<int>& pins)
{
    int kingPos = isWhiteTurn ? board[whitePlayer.positions[kingId]] : board[blackPlayer.positions[kingId]];

    //orthogonal:

    int x = (kingPos & 7) , y = (kingPos >> 3);

    while (board.InBounds(x, y))
    {
        for (unsigned int i = 0; i < 4; i++)
        {
            const Piece& piece = board.C_At(x, y);
            if (piece.Valid() && piece.isWhite != isWhiteTurn && (piece.type == PieceType::ROOK || piece.type == PieceType::QUEEN))
            {
                pins.push_back((y << 3) + x);
                break;
            }
        }
    }
}

void TurnState::SetCastlingIllegal(bool white)
{
    if (white)
    {
        castlingLegality[0] = false;
        castlingLegality[1] = false;
    }
    else
    {
        castlingLegality[2] = false;
        castlingLegality[3] = false;
    }
}

void TurnState::SetCastlingIllegal(bool white, bool right)
{
    if (white)
    {
        castlingLegality[right] = false;
    }
    else
    {
        castlingLegality[2 + right] = false;
    }
}

bool TurnState::GetCastlingLegal(bool white, bool right)
{
    return castlingLegality[(white ? 0 : 2) + right];
}

TurnState::TurnState()
{
    enpassantPosition = NO_ENPASSANT;
    capturedPiece = 0;
    castlingLegality[0] = true;
    castlingLegality[1] = true;
    castlingLegality[2] = true;
    castlingLegality[3] = true;
}



