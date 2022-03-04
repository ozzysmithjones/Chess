#include <limits>
#include <algorithm>
#include <assert.h>
#include "ChessPlayer.h"

//#include "Chess\Piece.h"

//using namespace std;

void ChessPlayer::setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, GameState* gameState)
{
	*playerBlack = new ChessPlayer(gameState, false);
	(*playerBlack)->SetAI(false, 0);

	*playerWhite = new ChessPlayer(gameState, true);
	(*playerWhite)->SetAI(false, 4);
}

ChessPlayer::ChessPlayer(GameState* _gameState, bool _isWhite)
{
	board = &_gameState->GetBoardRef();
	isWhite = _isWhite;
	gameState = _gameState;
}

void ChessPlayer::SetAI(bool _random, int _depth)
{
	isAI = true;
	random = _random;
	depth = _depth;
}

// chooseAIMove
// in this method - for an AI chess player - choose a move to make. This is called once per play. 
bool ChessPlayer::chooseAIMove(Move& moveToMake)
{
	std::vector<Move> moves = gameState->GetLegalMoves();
	if (moves.empty())
	{
		return false;
	}

	if (random)
	{
		moveToMake = moves[rand() % moves.size()];
		return true;
	}

	ScoreByZobristKey scoresByZobristKey;
	KillerMoveTable killerMoveTable;

	std::sort(moves.begin(), moves.end(), [this](const Move& a, const Move& b) { return this->PrioritiseMoveA(a, b, nullptr, 0); });
	moveToMake = moves[0];
	int bestScore = isWhite ? INT_MIN : INT_MAX;

	for (auto& move : moves)
	{
		//const uint64_t key = gameState->GetPositionZobristKey();

		gameState->MakeMove(move);
		int score = MiniMax(depth, gameState->IsWhiteTurn(), INT_MIN, INT_MAX, killerMoveTable);
		gameState->UnmakeMove();

		if ((score >= bestScore && isWhite) || (score <= bestScore && !isWhite))
		{
			bestScore = score;
			moveToMake = move;
		}
	}

	return true;
}

int ChessPlayer::MiniMax(int depth, bool white, int alpha, int beta, KillerMoveTable& killerMoveTable)
{
	std::vector<Move> moves = gameState->GetLegalMoves();
	//const uint64_t rootKey = gameState->GetPositionZobristKey();

	if ((depth <= 0) || moves.empty())
	{
		int score = EvaluatePosition(white, *board, moves);
		//scoresByZobristKey[rootKey] = PositionScore(PositionScoreType::EXACT, score, score);
		return score;
	}

	size_t numKillerMoves;
	const Move* killerMoves = killerMoveTable.GetKillerMoves(depth, numKillerMoves);
	std::sort(moves.begin(), moves.end(), [this, killerMoves, numKillerMoves](const Move& a, const Move& b) { return this->PrioritiseMoveA(a, b, killerMoves, numKillerMoves); });

	if (white)
	{
		int max = INT_MIN;

		for (auto move : moves)
		{
			gameState->MakeMove(move);

			int score = MiniMax(depth - 1, false, alpha, beta, killerMoveTable);
			max = std::max(max, score);
			gameState->UnmakeMove();

			if (max >= beta)
			{
				killerMoveTable.Push_front(depth, move);
				break;
			}

			alpha = std::max(alpha, max);
		}

		return max;
	}
	else
	{
		int min = INT_MAX;

		for (auto move : moves)
		{
			gameState->MakeMove(move);


			int score = MiniMax(depth - 1, true, alpha, beta, killerMoveTable);
			min = std::min(min, score);
			gameState->UnmakeMove();

			if (min <= alpha)
			{
				killerMoveTable.Push_front(depth, move);
				break;
			}

			beta = std::min(beta, min);
		}


		return min;
	}
}

bool ChessPlayer::PrioritiseMoveA(const Move& a, const Move& b, const Move* killerMoves, const size_t numKillerMoves) const
{
	//Prioritise "killer moves" that caused a cut-off before.
	for (size_t i = 0; i < numKillerMoves; i++)
	{
		if (killerMoves[i] == a)
		{
			return true;
		}
		else if (killerMoves[i] == b)
		{
			return false;
		}
	}

	{
		const bool aCheck = GetIsCheck(a);
		const bool bCheck = GetIsCheck(b);

		if (aCheck || bCheck)
		{
			return bCheck && !aCheck;
		}
	}

	{
		const PieceType aPromote = GetPromoteType(a);
		const PieceType bPromote = GetPromoteType(b);

		if (aPromote != PieceType::NONE || bPromote != PieceType::NONE)
		{
			return GetScore(aPromote) > GetScore(bPromote);
		}
	}

	PieceType aPiece = GetPieceType(a);
	PieceType bPiece = GetPieceType(b);
	PieceType aCapture = GetCaptureType(a);
	PieceType bCapture = GetCaptureType(b);

	if (aCapture != PieceType::NONE || bCapture != PieceType::NONE)
	{
		return (GetScore(aPiece) - GetScore(aCapture)) > (GetScore(bPiece) - GetScore(bCapture));
	}

	return CenterDiff(GetEndPos(a)) < CenterDiff(GetEndPos(b));
}

int ChessPlayer::EvaluatePosition(bool white, const Board& board, const std::vector<Move>& moves)
{
	if (moves.empty())
	{
		//return white ? INT_MIN : INT_MAX;

		if (gameState->IsInCheck())
		{
			return gameState->IsWhiteTurn() ? INT_MIN : INT_MAX;
		}
		else
		{
			return 0;
		}
	}

	return EvaluateSide(true, board) - EvaluateSide(false, board);
}


int ChessPlayer::EvaluateSide(bool white, const Board& board)
{
	//uint32_t maxPressure = 0;
	uint32_t score = 0;
	const unsigned int* initialPositions = white ? whiteStartingSquares : blackStartingSquares;
	const unsigned int* positions = white ? gameState->GetWhitePositions() : gameState->GetBlackPositions();
	const unsigned int promoteRow = white ? 7 : 0;
	GameStage stage = CalculateGameStage(white);

	unsigned int maxPawnAdvance = 0;

	for (unsigned int i = 0; i < 16; i++)
	{
		unsigned int position = positions[i];
		Piece piece = board[position];

		if (IsValid(piece) && GetId(piece) == i && IsWhite(piece) == white)
		{
			const PieceType pieceType = GetType(piece);
			score += (GetScore(piece) << 9);

			if (stage == GameStage::EARLY)
			{
				if (pieceType != PieceType::KING && pieceType != PieceType::PAWN && initialPositions[i] != position)
				{
					score += GetScore(piece);
				}
			}

			switch (pieceType)
			{
			case PieceType::PAWN:

				if (stage == GameStage::END)
				{
					maxPawnAdvance = std::max(abs(RowDiff(position, promoteRow)), (int)maxPawnAdvance);
				}
				else
				{
					score += (8 - (CenterDiff(position) << 1));
				}

				break;

			case PieceType::KING:

				if (stage == GameStage::END)
				{
					score -= CenterDiff(position, false);
				}
				else
				{
					score += CenterDiff(position, true);
				}

				break;

			default:

				score -= CenterDiff(position);
				break;
			}
		}
	}

	score += (maxPawnAdvance << 2);

	if (gameState->IsInCheck())
	{
		score += (gameState->IsWhiteTurn() == white ? -2 : 2);
	}

	return (int)score;
}


GameStage ChessPlayer::CalculateGameStage(bool white) const
{
	const unsigned int* squares = white ? gameState->GetWhitePositions() : gameState->GetBlackPositions();

	int startSimilarity = 0;
	int material = 0;

	for (unsigned int i = 0u; i < 16u; i++)
	{
		const Piece& p = board->operator[](squares[i]);
		if (IsValid(p) && GetId(p) == i && IsWhite(p) == white)
		{
			//Work out how much piece material the side has.

			if (GetType(p) != PieceType::PAWN && GetType(p) != PieceType::KING)
			{
				material += GetScore(p);
			}

			//work out how many of the piece positions are on starting squares.
			if (squares[i] == blackStartingSquares[i])
			{
				startSimilarity++;
			}
		}
	}

	if (startSimilarity >= 8)
	{
		return GameStage::EARLY;
	}

	if (material <= 9)
	{
		return GameStage::END;
	}

	return GameStage::MIDDLE;
}

int RowDiff(int position, int row)
{
	int r = (position >> 3);
	return abs(row - r);
}

int CenterDiff(int position, bool maximise)
{
	unsigned int x = position & 7;
	unsigned int y = position >> 3;

	const unsigned int centerMinX = 3;
	const unsigned int centerMinY = 3;
	const unsigned int centerMaxX = 4;
	const unsigned int centerMaxY = 4;

	int xDiff = std::min(abs((int)(centerMinX - x)), abs((int)(centerMaxX - x)));
	int yDiff = std::min(abs((int)(centerMinY - y)), abs((int)(centerMaxY - y)));

	return maximise ? std::max(xDiff, yDiff) : std::min(xDiff, yDiff); //(xDiff * xDiff) + (yDiff * yDiff); ///std::max(xDiff, yDiff);
}

int PosDiff(int position, int other)
{
	int xDiff = abs((int)((other & 7) - (position & 7)));
	int yDiff = abs((int)((other >> 3) - (position >> 3)));
	return std::max(xDiff, yDiff); //(xDiff * xDiff) + (yDiff * yDiff); ///std::max(xDiff, yDiff);
}

PositionScore::PositionScore()
{
}

PositionScore::PositionScore(PositionScoreType type, int score, int bound)
	: type(type), score(score), bound(bound)
{
}
