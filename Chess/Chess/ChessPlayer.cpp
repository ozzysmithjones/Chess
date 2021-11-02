#include <limits>
#include <algorithm>
#include "ChessPlayer.h"
//#include "Chess\Piece.h"

//using namespace std;

void ChessPlayer::setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, GameState* gameState)
{
	*playerBlack = new ChessPlayer(gameState, false);
	(*playerBlack)->SetAI(false,3);

	*playerWhite = new ChessPlayer(gameState,true);
	(*playerWhite)->SetAI(false,3);
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

unsigned int ChessPlayer::getAllLivePieces(vecPieces& vpieces)
{
	vpieces.clear();
	PieceInPostion pip;

	unsigned int count = 0;
	for (int i = board->MIN_ROW_INDEX; i < board->MAX_ROW_INDEX; i++)
	{
		for (int j = board->MIN_COL_INDEX; j < board->MAX_COL_INDEX; j++)
		{
			Piece pPiece = board->At(j,i);

			//if (pPiece == 0)
			//	continue;
		//	if (pPiece.isWhite != isWhite)
			//	continue;

			count++;
			pip.piece = pPiece;
			pip.row = i;
			pip.col = j;
			vpieces.emplace_back(pip);
		}
	}

	return count;
	
}

vector<Move> ChessPlayer::getValidMovesForPiece(PieceInPostion pip)
{
	return gameState->GetLegalMoves(pip.col, pip.row);//Gameplay::getValidMoves(m_pGameStatus, m_pBoard, pip.piece, pip.row, pip.col);
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

	std::sort(moves.begin(), moves.end(), [this](const Move& a, const Move& b) { return this->PrioritiseMoveA(a, b); });
	moveToMake = moves[0];
	int bestScore = isWhite ? INT_MIN : INT_MAX;

	for (auto& move : moves)
	{
		gameState->MakeMove(move);
		int score = MiniMax(depth, gameState->IsWhiteTurn(), INT_MIN, INT_MAX);
		gameState->UnmakeMove();

		if ((score >= bestScore && isWhite) || (score <= bestScore && !isWhite))
		{
			bestScore = score;
			moveToMake = move;
		}
	}

	return true;
}

int ChessPlayer::MiniMax(int depth, bool white, int alpha, int beta)
{
	std::vector<Move> moves = gameState->GetLegalMoves();

	if ((depth <= 0) || moves.empty())
	{
		return EvaluatePosition(white, *board, moves);
	}

	std::sort(moves.begin(), moves.end(), [this](const Move& a, const Move& b) { return this->PrioritiseMoveA(a, b); });
	if (white)
	{
		int max = INT_MIN;

		for (auto move : moves)
		{
			gameState->MakeMove(move);
			int score = MiniMax(depth - 1, false, alpha, beta);
			max = std::max(max, score);
			gameState->UnmakeMove();

			if (max >= beta)
			{
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
			int score = MiniMax(depth - 1, true, alpha, beta);
			min = std::min(min,score);
			gameState->UnmakeMove();

			if (min <= alpha)
			{
				break;
			}

			beta = std::min(beta, min);
		}

		return min;
	}
}

bool ChessPlayer::PrioritiseMoveA(const Move& a, const Move& b) const
{
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


	int score = 0;
	unsigned int* whitePositions = gameState->GetWhitePositions();;
	unsigned int* blackPositions = gameState->GetBlackPositions();;
	
	for (unsigned int i = 0; i < 16; i++)
	{
		unsigned int whitePosition = whitePositions[i];
		Piece piece = board[whitePosition];
		if (IsValid(piece) && IsWhite(piece) && GetId(piece) == i)
		{
			score += (GetScore(piece) << 4);

			switch (GetType(piece))
			{
			case PieceType::PAWN:
				score -= RowDiff(whitePosition, 7);
				break;
			case PieceType::KING:
				score += CenterDiff(whitePosition, false);
				break;

			default:
				score -= CenterDiff(whitePosition);;
				break;
			}

			OnPieceMoves(true, GetType(piece), whitePosition, gameState->GetStateLog(), board, [&score](unsigned int startPosition, unsigned int endPosition, MoveType moveType, PieceType capturedType)
				{
					score += capturedType != PieceType::NONE ? (GetScore(capturedType) << 1) : 1;
				});
		}		
	}

	for (unsigned int i = 0; i < 16; i++)
	{
		unsigned int blackPosition = blackPositions[i];
		Piece piece = board[blackPosition];
		if (IsValid(piece) && !IsWhite(piece) && GetId(piece) == i)
		{
			//score -= (GetScore(piece) << 3) - CenterDiff(blackPosition);

			score -= (GetScore(piece) << 4);

			switch (GetType(piece))
			{
			case PieceType::PAWN:
				score += RowDiff(blackPosition, 0);
				break;
			case PieceType::KING:
				score -= CenterDiff(blackPosition, false);
				break;

			default:
				score += CenterDiff(blackPosition);;
				break;
			}

			OnPieceMoves(false, GetType(piece), blackPosition, gameState->GetStateLog(), board, [&score](unsigned int startPosition, unsigned int endPosition, MoveType moveType, PieceType capturedType)
				{
					score -= capturedType != PieceType::NONE ? (GetScore(capturedType) << 1) : 1;
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
