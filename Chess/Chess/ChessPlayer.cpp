#include <limits>
#include <algorithm>
#include "ChessPlayer.h"
#include "GameState.h"
//#include "Chess\Piece.h"

//using namespace std;

void ChessPlayer::setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, GameState* gameState)
{
	*playerBlack = new ChessPlayer(gameState, false);
	(*playerBlack)->SetAI(false,2);

	*playerWhite = new ChessPlayer(gameState,true);
	(*playerWhite)->SetAI(false,2);
}

ChessPlayer::ChessPlayer(GameState* _gameState, bool _isWhite)
{
	board = &_gameState->GetBoard();
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

			if (pPiece == 0)
				continue;
			if (pPiece.isWhite != isWhite)
				continue;

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

	std::sort(moves.begin(), moves.end(), &MovePriority);
	moveToMake = moves[0];
	int bestScore = INT_MIN;

	for (auto& move : moves)
	{
		gameState->MakeMove(move);
		int score = MiniMax(depth, !isWhite, INT_MIN, INT_MAX) * (isWhite ? 1 : -1);
		gameState->UnmakeMove();

		if (score >= bestScore)
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

	/*
	bool containsCapture = false;
	for (auto& move : moves)
	{
		if (move.capturedPiece.data != 0)
		{
			containsCapture = true;
			break;
		}
	}
	*/

	if ((depth <= 0) || moves.empty())
	{
		return EvaluatePosition(white, *board,moves);
	}

	std::sort(moves.begin(), moves.end(), &MovePriority);
	if (white)
	{
		int max = INT_MIN;

		for (auto& move : moves)
		{
			gameState->MakeMove(move);
			max = std::max(max,MiniMax(depth-1, false,alpha,beta));
			alpha = std::max(alpha, max);
			gameState->UnmakeMove();
			
			if (beta <= alpha)
			{
				break;
			}
		}
		
		return max;
	}
	else 
	{
		int min = INT_MAX;

		for (auto& move : moves)
		{
			gameState->MakeMove(move);
			min = std::min(min,MiniMax(depth-1, true,alpha,beta));
			beta = std::min(beta, min);
			gameState->UnmakeMove();

			if (beta <= alpha)
			{
				break;
			}
		}

		return min;
	}
}

int ChessPlayer::EvaluatePosition(bool white, const Board& board, const std::vector<Move>& moves)
{
	if (moves.empty())
	{
		if (gameState->IsInCheck())
		{
			return white ? INT_MIN : INT_MAX;
		}
		else
		{
			return 0;
		}
	}
	
	/*
	int whiteKingPos = -1;
	int blackKingPos = -1;

	for (unsigned int i = 0; i < 64u; i++)
	{
		if (board.C_Index(i).Valid() && board.C_Index(i).type == PieceType::KING)
		{
			if (board.C_Index(i).isWhite)
			{
				whiteKingPos = i;
			}
			else 
			{
				blackKingPos = i;
			}

			if (blackKingPos != -1 && whiteKingPos != -1)
				break;
		}
	}
	*/

	int score = 0;
	for (unsigned int i = 0; i < 64u; i++)
	{
		if (board.C_Index(i).Valid())
		{
			const Piece& piece = board.C_Index(i);
			int m = (piece.isWhite ? 1 : -1);

			if (piece.type != PieceType::KING)
			{
				score += (Piece::GetMaterialValue(piece.type) - CenterDiff(i)) * m ; //   * m;
			}
			else
			{
				score += (Piece::GetMaterialValue(piece.type) + CenterDiff(i)) * m;
			}
		}
	}

	return score;
}

bool MovePriority(const Move& a, const Move& b)
{
	if ((a.endPiece.type != a.startPiece.type) || (b.endPiece.type != b.startPiece.type))
	{
		int aPow = (int)(Piece::GetMaterialValue(a.endPiece.type) - Piece::GetMaterialValue(a.startPiece.type));
		int bPow = (int)(Piece::GetMaterialValue(b.endPiece.type) - Piece::GetMaterialValue(b.startPiece.type));
		return aPow > bPow;
	}

	if (a.capturedPiece.data != 0 || b.capturedPiece.data != 0)
	{
		int aPow = (int)(Piece::GetMaterialValue(a.capturedPiece.type) - Piece::GetMaterialValue(a.endPiece.type));
		int bPow = (int)(Piece::GetMaterialValue(b.capturedPiece.type) - Piece::GetMaterialValue(b.endPiece.type));
		return aPow > bPow;
	}

	return  CenterDiff(a.endPosition) < CenterDiff(b.endPosition);
}

int CenterDiff(int position)
{
	unsigned int x = position & 7;
	unsigned int y = position >> 3;

	const unsigned int centerMinX = 3;
	const unsigned int centerMinY = 3;
	const unsigned int centerMaxX = 4;
	const unsigned int centerMaxY = 4;

	int xDiff = std::min(abs((int)(centerMinX - x)), abs((int)(centerMaxX - x)));
	int yDiff = std::min(abs((int)(centerMinY - y)), abs((int)(centerMaxY - y)));

	return std::max(xDiff, yDiff); //(xDiff * xDiff) + (yDiff * yDiff); ///std::max(xDiff, yDiff); //(xDiff * xDiff) + (yDiff * yDiff);
}

int PosDiff(int position, int other)
{
	int xDiff = abs((int)((other & 7) - (position & 7)));
	int yDiff = abs((int)((other >> 3) - (position >> 3)));
	return std::max(xDiff, yDiff); //(xDiff * xDiff) + (yDiff * yDiff); ///std::max(xDiff, yDiff);
}
