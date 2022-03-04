#include "GameState.h"
#include <iostream>


GameState::GameState()
{
	//zobristHasher = new ZobristHasher();
	stateLog.push(TurnState());
}

GameState::~GameState()
{
	//delete zobristHasher;
}

void GameState::MakeMove(const Move& move)
{
	//Get move variables: (they are compressed into an integer for space efficiency)
	unsigned int startPosition = GetStartPos(move);
	unsigned int endPosition = GetEndPos(move);
	MoveType moveType = GetMoveType(move);
	PieceType promoteType = GetPromoteType(move);

	//get piece variables: (they are compressed into an integer for space efficiency)
	Piece piece = board[startPosition];
	bool pieceIsWhite = IsWhite(piece);
	unsigned int pieceId = GetId(piece);
	PieceType pieceType = GetType(piece);

	if (pieceType == PieceType::KING)
	{
		if (pieceIsWhite)
		{
			whiteKingPos = endPosition;
		}
		else
		{
			blackKingPos = endPosition;
		}
	}

	//Create a new "turn state" struct to keep track of enpassant/castle ruling
	TurnState turnState = stateLog.top();
	turnState.enpassantPosition = NO_ENPASSANT;
	turnState.capturedPiece = board[endPosition];

	if (GetIsCheck(move))
	{
		turnState.checkPiecePosition = endPosition;
		turnState.checkPieceType = pieceType;
	}
	else
	{
		turnState.checkPiecePosition = 255;
		turnState.checkPieceType = PieceType::NONE;
	}

	//Move the piece:
	unsigned int* positions = pieceIsWhite ? whitePlayer.positions : blackPlayer.positions;
	positions[pieceId] = endPosition;
	board[endPosition] = piece;
	board[startPosition] = 0u;

	//castle directions:
	int left = pieceIsWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
	int right = left + 7;

	switch (moveType)
	{
	case MoveType::NORMAL:

		//castling made illegal
		if (pieceType == PieceType::KING && turnState.GetCastlingLegal(pieceIsWhite))
		{
			turnState.SetCastlingIllegal(pieceIsWhite);
		}

		if (pieceType == PieceType::ROOK && turnState.GetCastlingLegal(pieceIsWhite))
		{
			if (startPosition == right)
			{
				turnState.SetCastlingIllegal(pieceIsWhite, true);
			}
			else if (startPosition == left)
			{
				turnState.SetCastlingIllegal(pieceIsWhite, false);
			}
		}

		break;

	case MoveType::ADVANCED_PAWN:
		turnState.enpassantPosition = endPosition;
		break;

	case MoveType::ENPASSANT_LOWER:
		turnState.capturedPieceEnpassant = board[startPosition - 1];
		board[startPosition - 1] = 0u;
		break;
	case MoveType::ENPASSANT_HIGHER:
		turnState.capturedPieceEnpassant = board[startPosition + 1];
		board[startPosition + 1] = 0u;
		break;
	case MoveType::CASTLE_LOWER:

		{
			Piece rook = board[left];
			positions[GetId(rook)] = endPosition + 1;
			board[endPosition + 1] = rook;
			board[left] = 0u;
		}

		turnState.SetCastlingIllegal(pieceIsWhite);
		break;
	case MoveType::CASTLE_HIGHER:

		{
			Piece rook = board[right];
			positions[GetId(rook)] = endPosition - 1;
			board[endPosition - 1] = rook;
			board[right] = 0u;
		}

		turnState.SetCastlingIllegal(pieceIsWhite);
		break;

	case MoveType::PROMOTION:
		board[endPosition] = Promote(piece, promoteType);
		break;
	}

	moveLog.push(move);
	stateLog.push(turnState);
	isWhiteTurn = !isWhiteTurn;
}

void GameState::UnmakeMove()
{
	const Move& move = moveLog.top();
	moveLog.pop();

	//Get move variables: (they are compressed into an integer for space efficiency)
	unsigned int startPosition = GetStartPos(move);
	unsigned int endPosition = GetEndPos(move);
	MoveType moveType = GetMoveType(move);

	//get piece variables: (they are compressed into an integer for space efficiency)
	Piece piece = board[endPosition];
	bool pieceIsWhite = IsWhite(piece);
	unsigned int pieceId = GetId(piece);
	PieceType pieceType = GetType(piece);

	if (pieceType == PieceType::KING)
	{
		if (pieceIsWhite)
		{
			whiteKingPos = startPosition;
		}
		else
		{
			blackKingPos = startPosition;
		}
	}

	//Move the piece:
	unsigned int* positions = pieceIsWhite ? whitePlayer.positions : blackPlayer.positions;
	positions[pieceId] = startPosition;
	board[startPosition] = piece;
	board[endPosition] = stateLog.top().capturedPiece;

	//castle directions:
	int left = pieceIsWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
	int right = left + 7;

	switch (moveType)
	{
	case MoveType::NORMAL:
		break;

	case MoveType::ADVANCED_PAWN:
		break;

	case MoveType::ENPASSANT_LOWER:
		board[startPosition - 1] = stateLog.top().capturedPieceEnpassant;
		break;
	case MoveType::ENPASSANT_HIGHER:
		board[startPosition + 1] = stateLog.top().capturedPieceEnpassant;
		break;
	case MoveType::CASTLE_LOWER:

		{
			Piece rook = board[endPosition + 1];
			positions[GetId(rook)] = left;
			board[left] = rook;
			board[endPosition + 1] = 0u;
		}

		break;

	case MoveType::CASTLE_HIGHER:

		{
			Piece rook = board[endPosition - 1];
			positions[GetId(rook)] = right;
			board[right] = rook;
			board[endPosition - 1] = 0u;
		}

		break;

	case MoveType::PROMOTION:
		board[startPosition] = Promote(piece, PieceType::PAWN);
		break;
	}

	//shift turn
	isWhiteTurn = !isWhiteTurn;
	stateLog.pop();
}

uint64_t GameState::CalculateZobristKey() const
{
	return 0; //zobristHasher->HashForPosition(isWhiteTurn, board, stateLog.top(), whitePlayer.positions, blackPlayer.positions);
}

bool GameState::IsInCheck()
{
	return IsInCheck(stateLog.top(), isWhiteTurn);
}

void GameState::GetAvalibleMoves(std::vector<Move>& moves, unsigned int position, Piece piece)
{
	bool isWhite = IsWhite(piece);
	switch (GetType(piece))
	{
	case PieceType::PAWN:
		AddPawnMoves(isWhite, position, stateLog, board, moves);
		break;

	case PieceType::KNIGHT:
		AddKnightMoves(isWhite, position, board, moves);
		break;

	case PieceType::BISHOP:
		AddBishopMoves(isWhite, position, board, moves, false);
		break;

	case PieceType::ROOK:
		AddRookMoves(isWhite, position, board, moves, false);
		break;

	case PieceType::QUEEN:
		AddBishopMoves(isWhite, position, board, moves, true);
		AddRookMoves(isWhite, position, board, moves, true);
		break;

	case PieceType::KING:
		AddKingMoves(isWhite, position, stateLog, board, moves);
		break;
	}
}

void GameState::GetAvalibleMoves(std::vector<Move>& moves)
{
	Player& player = isWhiteTurn ? whitePlayer : blackPlayer;

	//reserve moves:
	unsigned int count = 0;
	for (unsigned int i = 0; i < 16u; i++)
	{
		unsigned int position = player.positions[i];
		const Piece piece = board[position];
		bool isWhite = IsWhite(piece);

		if (IsValid(piece) && isWhite == isWhiteTurn && GetId(piece) == i)
		{
			OnPieceMoves(isWhite, GetType(piece), position, stateLog, board, [&count](unsigned int startPos, unsigned int endPos, MoveType moveType, PieceType captureType) { count++; });
		}
	}

	moves.reserve(std::min(count, moves.max_size()));

	for (unsigned int i = 0; i < 16u; i++)
	{
		const Piece piece = board[player.positions[i]];
		if (IsValid(piece) && IsWhite(piece) == isWhiteTurn && GetId(piece) == i)
		{
			GetAvalibleMoves(moves, player.positions[i], piece);
		}
	}
}

bool GameState::IsInCheck(const TurnState& turnState, bool isWhite)
{
	/*
	unsigned pos = turnState.checkPiecePosition;
	unsigned x = pos & 7;
	unsigned y = pos >> 3;

	if (turnState.checkPieceType != PieceType::NONE && (!IsValid(board[pos]) || GetType(board[pos]) != turnState.checkPieceType || IsWhite(board[pos]) == isWhite))
	{
		return false;
	}

	switch (turnState.checkPieceType)
	{
	case PieceType::NONE: break;
	case PieceType::PAWN: return PawnDeliversCheck(pos, isWhite);
	case PieceType::KNIGHT: return KnightDeliversCheck(x, y, isWhite);
	case PieceType::BISHOP: return BishopDeliversCheck(x, y, isWhite);
	case PieceType::ROOK: return RookDeliversCheck(x, y, isWhite);
	case PieceType::QUEEN: return BishopDeliversCheck(x, y, isWhite) || RookDeliversCheck(x, y, isWhite);
	case PieceType::KING: return KingDeliversCheck(x, y, isWhite);
	default:
		break;
	}
	*/


	const unsigned int kingPos = isWhite ? whiteKingPos : blackKingPos;
	const unsigned int kingX = (kingPos & 7);
	const unsigned int kingY = (kingPos >> 3);

	unsigned pos;
	unsigned x;
	unsigned y;

	//Check orthogonal and diagonal moves;
	for (unsigned int i = 0; i < 4u; i++)
	{
		int xOffset = orthoX[i];
		int yOffset = orthoY[i];
		x = kingX + xOffset;
		y = kingY + yOffset;

		while (board.InBounds(x, y))
		{
			pos = ToIndex(x, y);
			if (IsValid(board[pos]))
			{
				bool isPieceWhite = IsWhite(board[pos]);
				PieceType type = GetType(board[pos]);

				if ((isPieceWhite != isWhite) && (type == PieceType::ROOK || type == PieceType::QUEEN))
				{
					return true;
				}

				break;
			}

			x += xOffset;
			y += yOffset;
		}

		xOffset = diagX[i];
		yOffset = diagY[i];
		x = kingX + xOffset;
		y = kingY + yOffset;

		while (board.InBounds(x, y))
		{
			pos = ToIndex(x, y);

			if (IsValid(board[pos]))
			{
				bool isPieceWhite = IsWhite(board[pos]);
				PieceType type = GetType(board[pos]);

				if ((isPieceWhite != isWhite) && (type == PieceType::BISHOP || type == PieceType::QUEEN))
				{
					return true;
				}

				break;
			}

			x += xOffset;
			y += yOffset;
		}
	}


	//Check knight moves:

	for (unsigned int i = 0; i < 8u; i++)
	{
		x = kingX + ((i & 1u) != 0u ? 2 : 1) * ((i & 2u) != 0u ? 1 : -1);
		y = kingY + ((i & 1u) == 0u ? 2 : 1) * ((i & 4u) != 0u ? 1 : -1);
		pos = ToIndex(x, y);

		if (board.InBounds(x, y) && (IsValid(board[pos] && IsWhite(board[pos]) != isWhite && GetType(board[pos]) == PieceType::KNIGHT)))
		{
			return true;
		}
	}

	//check pawn moves:

	const int pawnOffset = isWhite ? 1 : -1;

	if (kingX != 0) // check left side for pawn
	{
		x = kingX - 1;
		y = kingY + pawnOffset;
		pos = ToIndex(x, y);

		if (IsValid(board[pos]) && IsWhite(board[pos]) != isWhite && GetType(board[pos]) == PieceType::PAWN)
		{
			return true;
		}
	}

	if (kingX != 7)
	{
		x = kingX + 1;
		y = kingY + pawnOffset;
		pos = ToIndex(x, y);

		if (IsValid(board[pos]) && IsWhite(board[pos]) != isWhite && GetType(board[pos]) == PieceType::PAWN)
		{
			return true;
		}
	}

	//check king moves:
	for (int yi = -1; yi <= 1; yi++)
	{
		for (int xi = -1; xi <= 1; xi++)
		{
			x = kingX + xi;
			y = kingY + yi;
			pos = ToIndex(x, y);

			if (board.InBounds(x, y) && IsWhite(board[pos]) != isWhite && GetType(board[pos]) == PieceType::KING)
			{
				return true;
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
	PieceType pieces[] = {
		PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
		PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK
	};

	//Place pieces down:
	for (unsigned int x = Board::MIN_COL_INDEX; x < Board::MAX_ROW_INDEX; x++)
	{
		board.At(x, firstRow) = CreatePiece(white, pieces[x], x);
		board.At(x, secondRow) = CreatePiece(white, PieceType::PAWN, x + 8);
		positions[x] = (firstRow << 3) + x;
		positions[x + 8] = (secondRow << 3) + x;
	}

	unsigned& kingPos = white ? whiteKingPos : blackKingPos;
	kingPos = positions[4];

	//zobristKey = zobristHasher->HashForPosition(true, board, TurnState(), whitePlayer.positions, blackPlayer.positions);
	//stateLog.top().zob = zobristKey;
}

std::vector<Move> GameState::GetLegalMoves()
{
	std::vector<Move> moves;
	GetAvalibleMoves(moves);

	const TurnState& turnState = stateLog.top();
	const bool inCheck = turnState.checkPieceType != PieceType::NONE;

	for (size_t i = 0; i < moves.size(); i++)
	{
		//if (inCheck || GetPieceType(moves[i]) == PieceType::KING)
		//{

		//}

		MakeMove(moves[i]);

		if (IsInCheck(turnState, !isWhiteTurn))
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

	const TurnState& turnState = stateLog.top();
	const bool inCheck = turnState.checkPieceType != PieceType::NONE;

	for (int i = 0; i < moves.size(); i++)
	{
		//if (inCheck || GetPieceType(moves[i]) == PieceType::KING)
		//{

		//}

		MakeMove(moves[i]);

		if (IsInCheck(turnState, !isWhiteTurn))
		{
			moves[i] = moves[moves.size() - 1];
			moves.pop_back();
			i--;
		}

		UnmakeMove();
	}

	return moves;
}

/*
void GameState::GetPawnMoves(std::vector<Move>& moves, unsigned int position, Piece pawn)
{
    bool isWhite = IsWhite(pawn);
    int direction = isWhite ? 8 : -8;
    unsigned int forward = position + direction;

    bool rightInBounds = board.InBounds((forward & 7) + 1, forward >> 3);
    bool leftInBounds = board.InBounds((forward & 7) - 1, forward >> 3);

    if (rightInBounds)
    {
        //Normal diagonal capture.

        if (board[forward + 1] != 0 && IsWhite(board[forward + 1]) != isWhite)
        {
            AddPawnMove(moves, CreateMove(positions,forwa) Move(MoveType::NORMAL,position, forward + 1),isWhite);
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

    for (unsigned int i = 0; i < 8u; i++)
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
*/


void GameState::AddPawnMoves(bool whitePiece, unsigned int position, const std::stack<TurnState>& stateLog, const Board& board, std::vector<Move>& moves)
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

	MoveType defaultType = ((position + direction) >> 3) != promoteRow ? MoveType::NORMAL : MoveType::PROMOTION;

	//forward move:

	if (board[position + direction] == 0)
	{
		AddPawnMove(moves, CreateMove(position, position + direction, defaultType, PieceType::PAWN, PieceType::NONE, PawnDeliversCheck(position + direction, !whitePiece)), whitePiece);

		if (y == advanceRow && board[position + direction + direction] == 0)
		{
			moves.push_back(CreateMove(position, position + direction + direction, MoveType::ADVANCED_PAWN, PieceType::PAWN, PieceType::NONE, PawnDeliversCheck(position + direction + direction, !whitePiece)));
		}
	}

	//diagonal capture

	if (x != 7)
	{
		if (board[position + direction + 1] != 0 && IsWhite(board[position + direction + 1]) != whitePiece)
		{
			AddPawnMove(moves, CreateMove(position, position + direction + 1, defaultType, PieceType::PAWN, GetType(board[position + direction + 1]), PawnDeliversCheck(position + direction + 1, !whitePiece)), whitePiece);
		}

		if (position + 1 == enPassantablePosition && board[position + direction + 1] == 0 && IsWhite(board[position + 1]) != whitePiece)
		{
			moves.push_back(CreateMove(position, position + direction + 1, MoveType::ENPASSANT_HIGHER, PieceType::PAWN, PieceType::PAWN, PawnDeliversCheck(position + direction + 1, !whitePiece)));
		}
	}

	if (x != 0)
	{
		if (board[position + direction - 1] != 0 && IsWhite(board[position + direction - 1]) != whitePiece)
		{
			AddPawnMove(moves, CreateMove(position, position + direction - 1, defaultType, PieceType::PAWN, GetType(board[position + direction + 1]), PawnDeliversCheck(position + direction - 1, !whitePiece)), whitePiece);
		}

		if (position - 1 == enPassantablePosition && board[position + direction - 1] == 0 && IsWhite(board[position - 1]) != whitePiece)
		{
			moves.push_back(CreateMove(position, position + direction - 1, MoveType::ENPASSANT_LOWER, PieceType::PAWN, PieceType::PAWN, PawnDeliversCheck(position + direction - 1, !whitePiece)));
		}
	}
}

void GameState::AddKnightMoves(bool isWhite, unsigned int position, const Board& board, std::vector<Move>& moves)
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
			moves.push_back(CreateMove(position, endPosition, MoveType::NORMAL, PieceType::KNIGHT, GetType(board[endPosition]), KnightDeliversCheck(x, y, !isWhite)));
		}
	}
}

void GameState::AddBishopMoves(bool isWhite, unsigned int position, const Board& board, std::vector<Move>& moves, bool isQueen)
{
	PieceType pieceType = isQueen ? PieceType::QUEEN : PieceType::BISHOP;
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
					moves.push_back(CreateMove(position, endPosition, MoveType::NORMAL, pieceType, GetType(board[endPosition]), BishopDeliversCheck(x, y, !isWhite)));
				}

				break;
			}

			moves.push_back(CreateMove(position, endPosition, MoveType::NORMAL, pieceType, PieceType::NONE, BishopDeliversCheck(x, y, !isWhite)));

			x += xDelta;
			y += yDelta;
			endPosition = ToIndex(x, y);
		}
	}
}

void GameState::AddRookMoves(bool isWhite, unsigned int position, const Board& board, std::vector<Move>& moves, bool isQueen)
{
	PieceType pieceType = isQueen ? PieceType::QUEEN : PieceType::ROOK;
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
					moves.push_back(CreateMove(position, endPosition, MoveType::NORMAL, pieceType, GetType(board[endPosition]), RookDeliversCheck(x, y, !isWhite)));
				}

				break;
			}

			moves.push_back(CreateMove(position, endPosition, MoveType::NORMAL, pieceType, PieceType::NONE, RookDeliversCheck(x, y, !isWhite)));

			x += xDelta;
			y += yDelta;
			endPosition = ToIndex(x, y);
		}
	}
}

void GameState::AddKingMoves(bool isWhite, unsigned int position, const std::stack<TurnState>& stateLog, const Board& board, std::vector<Move>& moves)
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
				moves.push_back(CreateMove(position, endPosition, MoveType::NORMAL, PieceType::KING, GetType(board[endPosition]), KingDeliversCheck(centerX + x, centerY + y, !isWhite)));
			}
		}
	}

	//castle:

	bool leftLegal = true;
	bool rightLegal = true;
	unsigned int left = isWhite ? 0 : (Board::WIDTH * Board::HEIGHT) - 8;
	unsigned int right = left + 7;

	if (!stateLog.empty())
	{
		leftLegal = stateLog.top().GetCastlingLegal(isWhite, false);
		rightLegal = stateLog.top().GetCastlingLegal(isWhite, true);
	}

	if (leftLegal && board[left] != 0 && GetType(board[left]) == PieceType::ROOK && IsWhite(board[left]) == isWhite && board[position - 1] == 0 && board[position - 2] == 0)
	{
		const bool kingChecks = KingDeliversCheck((position - 2) & 7, (position - 2) >> 3, !isWhite);
		const bool rookChecks = RookDeliversCheck((position - 1) & 7, (position - 1) >> 3, !isWhite);
		moves.push_back(CreateMove(position, position - 2, MoveType::CASTLE_LOWER, PieceType::KING, PieceType::NONE, kingChecks || rookChecks));
	}

	if (rightLegal && board[right] != 0 && GetType(board[right]) == PieceType::ROOK && IsWhite(board[right]) == isWhite && board[position + 1] == 0 && board[position + 2] == 0)
	{
		const bool kingChecks = KingDeliversCheck((position + 2) & 7, (position + 2) >> 3, !isWhite);
		const bool rookChecks = RookDeliversCheck((position + 1) & 7, (position + 1) >> 3, !isWhite);
		moves.push_back(CreateMove(position, position + 2, MoveType::CASTLE_HIGHER, PieceType::KING, PieceType::NONE, kingChecks || rookChecks));
	}
}

bool GameState::PawnDeliversCheck(const unsigned int position, bool kingIsWhite, PieceType promoteType) const
{
	/*
	if (promoteType != PieceType::NONE)
	{
		const unsigned int x = position & 7;
		const unsigned int y = position >> 3;

		switch (promoteType)
		{
		case PieceType::KNIGHT: return KnightDeliversCheck(x,y, kingIsWhite);
		case PieceType::BISHOP: return BishopDeliversCheck(x, y, kingIsWhite);
		case PieceType::ROOK: return RookDeliversCheck(x, y, kingIsWhite);
		case PieceType::QUEEN: return BishopDeliversCheck(x, y, kingIsWhite) || RookDeliversCheck(x, y, kingIsWhite);
		default: return false;
		}
	}
	*/

	const unsigned& kingPos = kingIsWhite ? whiteKingPos : blackKingPos;
	const int forward = kingIsWhite ? -8 : 8;
	const unsigned int kingX = (kingPos & 7);

	return (position == (kingPos + forward + 1) && kingX != 7) || (position == (kingPos + forward - 1) && kingX != 0); // check left side for pawn
}

bool GameState::BishopDeliversCheck(unsigned int x, unsigned int y, bool kingIsWhite) const
{
	const unsigned& kingPos = kingIsWhite ? whiteKingPos : blackKingPos;
	const unsigned int kingX = (kingPos & 7);
	const unsigned int kingY = (kingPos >> 3);

	int xDiff = (int)(kingX - x);
	int yDiff = (int)(kingY - y);
	if (abs(xDiff) == abs(yDiff))
	{
		xDiff = std::max(-1, std::min(xDiff, 1));
		yDiff = std::max(-1, std::min(yDiff, 1));
		x += xDiff;
		y += yDiff;

		while (x != kingX && y != kingY)
		{
			if (board[(y << 3) + x] != 0)
			{
				return false;
			}

			x += xDiff;
			y += yDiff;
		}

		return true;
	}

	return false;
}

bool GameState::RookDeliversCheck(unsigned int x, unsigned int y, bool kingIsWhite) const
{
	const unsigned& kingPos = kingIsWhite ? whiteKingPos : blackKingPos;
	const unsigned int kingX = (kingPos & 7);
	const unsigned int kingY = (kingPos >> 3);

	if (x == kingX || y == kingY)
	{
		int xDiff = (int)(kingX - x);
		int yDiff = (int)(kingY - y);
		xDiff = std::max(-1, std::min(xDiff, 1));
		yDiff = std::max(-1, std::min(yDiff, 1));

		x += xDiff;
		y += yDiff;

		while (x != kingX || y != kingY)
		{
			if (board[(y << 3) + x] != 0)
			{
				return false;
			}

			x += xDiff;
			y += yDiff;
		}

		return true;
	}

	return false;
}

bool GameState::KnightDeliversCheck(const unsigned int x, const unsigned int y, bool kingIsWhite) const
{
	const unsigned& kingPos = kingIsWhite ? whiteKingPos : blackKingPos;
	const unsigned int kingX = (kingPos & 7);
	const unsigned int kingY = (kingPos >> 3);

	const int xDiff = abs((int)(x - kingX));
	const int yDiff = abs((int)(y - kingY));

	return (xDiff == 1 && yDiff == 2) || (xDiff == 2 && yDiff == 1);
}

bool GameState::KingDeliversCheck(const unsigned int x, const unsigned int y, bool kingIsWhite) const
{
	const unsigned& kingPos = kingIsWhite ? whiteKingPos : blackKingPos;
	const unsigned int kingX = (kingPos & 7);
	const unsigned int kingY = (kingPos >> 3);

	const int xDiff = abs((int)(x - kingX));
	const int yDiff = abs((int)(y - kingY));

	return (xDiff <= 1 && yDiff <= 1);
}


void GameState::AddPawnMove(std::vector<Move>& moves, Move move, bool isWhite)
{
	if (GetMoveType(move) == MoveType::PROMOTION)
	{
		moves.emplace_back(SetPromoteMove(move, PieceType::KNIGHT));
		moves.emplace_back(SetPromoteMove(move, PieceType::BISHOP));
		moves.emplace_back(SetPromoteMove(move, PieceType::ROOK));
		moves.emplace_back(SetPromoteMove(move, PieceType::QUEEN));
	}
	else
	{
		moves.push_back(move);
	}
}
