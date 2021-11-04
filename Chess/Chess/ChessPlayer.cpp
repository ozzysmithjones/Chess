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

	*playerWhite = new ChessPlayer(gameState,true);
	(*playerWhite)->SetAI(false, 0);
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

	std::unordered_map<uint64_t, PositionScore> scoresByPosition;
	KillerMoveTable killerMoveTable;

	std::sort(moves.begin(), moves.end(), [this](const Move& a, const Move& b) { return this->PrioritiseMoveA(a, b,nullptr,0); });
	moveToMake = moves[0];
	int bestScore = isWhite ? INT_MIN : INT_MAX;

	for (auto& move : moves)
	{
		const uint64_t key = gameState->GetPositionZobristKey();
		gameState->MakeMove(move);
		int score = MiniMax(depth, gameState->IsWhiteTurn(), INT_MIN, INT_MAX, killerMoveTable, scoresByPosition);
		gameState->UnmakeMove();

		//bool exact = ;
		//assert(exact);
		if (key != gameState->GetPositionZobristKey())
		{
			//Get move variables: (they are compressed into an integer for space efficiency)
			unsigned int startPosition = GetStartPos(move);
			unsigned int endPosition = GetEndPos(move);
			MoveType moveType = GetMoveType(move);
			PieceType captureType = GetCaptureType(move);
			PieceType promoteType = GetPromoteType(move);

			Piece piece = gameState->GetBoard()[startPosition];
			bool pieceIsWhite = IsWhite(piece);
			unsigned int pieceId = GetId(piece);
			PieceType pieceType = GetType(piece);

			int x = endPosition & 7;
			int y = endPosition >> 3;
			int num = 0;
		}

		if ((score >= bestScore && isWhite) || (score <= bestScore && !isWhite))
		{
			bestScore = score;
			moveToMake = move;
		}
	}

	return true;
}

int ChessPlayer::MiniMax(int depth, bool white, int alpha, int beta, KillerMoveTable& killerMoveTable, ScoreByZobristKey& scoresByZobristKey)
{
	std::vector<Move> moves = gameState->GetLegalMoves();
	const uint64_t rootKey = gameState->GetPositionZobristKey();

	if ((depth <= 0) || moves.empty())
	{
		
		int score = EvaluatePosition(white, *board, moves);
		scoresByZobristKey[rootKey].type = PositionScoreType::EXACT;
		scoresByZobristKey[rootKey].bound = score;
		scoresByZobristKey[rootKey].score = score;
		return score;
	}

	size_t numKillerMoves;
	const Move* killerMoves = killerMoveTable.GetKillerMoves(depth, numKillerMoves);
	std::sort(moves.begin(), moves.end(), [this, killerMoves, numKillerMoves](const Move& a, const Move& b) { return this->PrioritiseMoveA(a, b,killerMoves,numKillerMoves); });

	if (white)
	{
		int max = INT_MIN;

		for (auto move : moves)
		{
			gameState->MakeMove(move);

			const uint64_t key = gameState->GetPositionZobristKey();
			if (scoresByZobristKey.find(key) != scoresByZobristKey.end())
			{
				const PositionScore& positionScore = scoresByZobristKey[key];

				if ((positionScore.type == PositionScoreType::EXACT) || (positionScore.type == PositionScoreType::ALPHA && positionScore.bound >= beta))
				{
					gameState->UnmakeMove();
					return scoresByZobristKey[key].score;
				}
			}

			int score = MiniMax(depth - 1, false, alpha, beta, killerMoveTable, scoresByZobristKey);
			max = std::max(max, score);
			gameState->UnmakeMove();

			if (max >= beta)
			{
				killerMoveTable.Push_front(depth, move);
				break;
			}

			alpha = std::max(alpha, max);
		}
		
		scoresByZobristKey[rootKey].type = PositionScoreType::ALPHA;
		scoresByZobristKey[rootKey].bound = beta;
		scoresByZobristKey[rootKey].score = max;
		return max;
	}
	else 
	{
		int min = INT_MAX;

		for (auto move : moves)
		{
			gameState->MakeMove(move);

			const uint64_t key = gameState->GetPositionZobristKey();
			if (scoresByZobristKey.find(key) != scoresByZobristKey.end())
			{
				const PositionScore& positionScore = scoresByZobristKey[key];
				if ((positionScore.type == PositionScoreType::EXACT) || (positionScore.type == PositionScoreType::BETA && positionScore.bound >= alpha))
				{
					gameState->UnmakeMove();
					return scoresByZobristKey[key].score;
				}
			}

			int score = MiniMax(depth - 1, true, alpha, beta, killerMoveTable, scoresByZobristKey);
			min = std::min(min,score);
			gameState->UnmakeMove();

			if (min <= alpha)
			{
				killerMoveTable.Push_front(depth, move);
				break;
			}

			beta = std::min(beta, min);
		}

		scoresByZobristKey[rootKey].type = PositionScoreType::BETA;
		scoresByZobristKey[rootKey].bound = alpha;
		scoresByZobristKey[rootKey].score = min;
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
	

	PieceType aPromote = GetPromoteType(a);
	PieceType bPromote = GetPromoteType(b);

	if (aPromote != PieceType::NONE || bPromote != PieceType::NONE)
	{
		return GetScore(aPromote) > GetScore(bPromote);
	}

	PieceType aCapture = GetCaptureType(a);
	PieceType bCapture = GetCaptureType(b);

	if (aCapture != PieceType::NONE || bCapture != PieceType::NONE)
	{
		return GetScore(aCapture) > GetScore(bCapture);
	}

	return CenterDiff(GetEndPos(a)) < CenterDiff(GetEndPos(b));
}

int ChessPlayer::EvaluatePosition(bool white, const Board& board, const std::vector<Move>& moves)
{

	if (moves.empty())
	{
		//return white ? INT_MIN : INT_MAX;
		
		if (gameState->IsInCheck(white))
		{
			return white ? INT_MIN : INT_MAX;
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
	int score = 0;
	const unsigned int* positions = white ? gameState->GetWhitePositions() : gameState->GetBlackPositions();
	const unsigned int promoteRow = white ? 7 : 0;

	for (unsigned int i = 0; i < 16; i++)
	{
		unsigned int position = positions[i];
		Piece piece = board[position];
		if (IsValid(piece) && IsWhite(piece) == white && GetId(piece) == i)
		{
			score += (GetScore(piece) << 5);

			switch (GetType(piece))
			{
			case PieceType::PAWN:
				score -= (RowDiff(position, promoteRow));
				break;
			case PieceType::KING:
				score += CenterDiff(position, false) << 1;
				break;

			default:
				score -= CenterDiff(position) << 1;
				break;
			}

			
			OnPieceMoves(white, GetType(piece), position, gameState->GetStateLog(), board, [&score](unsigned int startPosition, unsigned int endPosition, MoveType moveType, PieceType capturedType)
				{
					score += capturedType != PieceType::NONE ? GetScore(capturedType) : 1;
				});
				
		}
	}

	return score;
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

	return maximise ? std::max(xDiff, yDiff) : std::min(xDiff, yDiff);//(xDiff * xDiff) + (yDiff * yDiff); ///std::max(xDiff, yDiff);
}

int PosDiff(int position, int other)
{
	int xDiff = abs((int)((other & 7) - (position & 7)));
	int yDiff = abs((int)((other >> 3) - (position >> 3)));
	return std::max(xDiff, yDiff); //(xDiff * xDiff) + (yDiff * yDiff); ///std::max(xDiff, yDiff);
}
