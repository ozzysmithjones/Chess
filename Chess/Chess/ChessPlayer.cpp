#include <limits>
#include <algorithm>
#include "ChessPlayer.h"


void ChessPlayer::setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, Chess* chess)
{
	*playerBlack = new ChessPlayer(chess, false);
	(*playerBlack)->SetAI(false, 3);

	*playerWhite = new ChessPlayer(chess, true);
	//(*playerWhite)->SetAI(true, 1);
}

ChessPlayer::ChessPlayer(Chess* chess, bool isWhite)
	: chess(chess), isWhite(isWhite)
{

}

void ChessPlayer::SetAI(bool _random, int _depth)
{
	isAI = true;
	random = _random;
	depth = _depth;
}



// chooseAIMove
// in this method - for an AI chess player - choose a move to make. This is called once per play. 
bool ChessPlayer::chooseAIMove(std::vector<Move>& moves, Move& moveToMake)
{
	if (moves.empty())
	{
		return false;
	}

	if (random)
	{
		moveToMake = moves[rand() % moves.size()];
		return true;
	}

	//std::sort(moves.begin(), moves.end(), [this](const Move& a, const Move& b) { return this->PrioritiseMoveA(a, b); });
	moveToMake = moves[0];
	int bestScore = INT_MIN;

	for (auto& move : moves)
	{
		chess->MakeMove(move);
		int score = -NegaMax(depth, INT_MIN, INT_MAX);
		chess->UndoMove();

		if (score >= bestScore)
		{
			bestScore = score;
			moveToMake = move;
		}
	}

	return true;
}

void ChessPlayer::PrioritiseMoves(std::vector<Move>& moves)
{
	//64 is maximum priority (6 bits)

	for (auto& move : moves)
	{
		unsigned prio = 0u;

		prio |= ((uint32_t)GetIsCheck(prio) << 6);
		prio |= (uint32_t)(GetCaptureType(prio) != PieceType::none) << 5;
		prio |= (uint32_t)(GetPieceType(prio) != PieceType::pawn) << 4;
		prio += (4u - CenterDiff(GetEndPosition(move)));

		SetPriority(move, prio);
	}
}

int ChessPlayer::NegaMax(int depth, int alpha, int beta)
{
	if (depth == 0)
	{
		return EvaluatePosition();
	}

	std::vector<Move> moves;
	chess->CalculateLegalMoves(moves);

	if (moves.empty())
	{
		if (chess->IsCheck())
		{
			return INT_MIN;
		}
		else
		{
			return 0;
		}
	}

	PrioritiseMoves(moves);
	std::sort(moves.begin(), moves.end(), [](const Move& a, const Move& b) -> bool { return GetPriority(a) > GetPriority(b); });
	int best = std::numeric_limits<int>::min();

	for (const auto& move : moves)
	{
		chess->MakeMove(move);

		int score = -NegaMax(depth - 1, -beta, -alpha);

		if (score > best)
		{
			best = score;
		}

		if (score > alpha)
		{
			alpha = score;
		}

		chess->UndoMove();

		if (alpha >= beta)
		{
			return alpha;
		}
	}

	return best;
}

const int pieceValues[]{ 1, 3, 3, 5, 9, 64 };

int ChessPlayer::EvaluatePosition()
{
	int score = 0;
	const bool isWhite = chess->IsWhiteTurn();
	const GameState& state = chess->GetGameState();

	//Count Material:

	for (std::size_t i = 0; i < 6; i++)
	{
		int numFor = GetBitCount(state.pieces[i] & state.occupancy[isWhite]);
		int numAgaisnt = GetBitCount(state.pieces[i] & state.occupancy[!isWhite]);

		score += (numFor * pieceValues[i] - numAgaisnt * pieceValues[i]) << 6;
	}

	//Count Centre control:

	for (std::size_t i = 0; i < 6; i++)
	{
		int numFor = GetBitCount(state.pieces[i] & state.occupancy[isWhite] & centre);
		int numAgaisnt = GetBitCount(state.pieces[i] & state.occupancy[!isWhite] & centre);

		score += numFor - numAgaisnt;
	}

	//Count influence of pieces:

	const uint64_t occupancy = state.occupancy[0] | state.occupancy[1];

	for (std::size_t i = 0; i < 6; i++)
	{
		for (uint64_t pieces = state.pieces[i] & state.occupancy[isWhite]; pieces != 0;)
		{
			unsigned square = GetLeastIndex(pieces);

			switch ((PieceType)(i + 1))
			{
			case PieceType::pawn:
				break;
			case PieceType::knight:
				score += GetBitCount(chess->GetKnightAttacks(square));
				break;
			case PieceType::bishop:
				score += GetBitCount(chess->GetBishopAttacks(square,occupancy));
				break;
			case PieceType::rook:
				score += GetBitCount(chess->GetRookAttacks(square, occupancy));
				break;
			case PieceType::queen:
				score += GetBitCount(chess->GetBishopAttacks(square, occupancy));
				score += GetBitCount(chess->GetRookAttacks(square, occupancy));
				break;
			}

			PopBit(pieces, square);
		}

		for (uint64_t pieces = state.pieces[i] & state.occupancy[!isWhite]; pieces != 0;)
		{
			unsigned square = GetLeastIndex(pieces);

			switch ((PieceType)(i + 1))
			{
			case PieceType::pawn:
				break;
			case PieceType::knight:
				score -= GetBitCount(chess->GetKnightAttacks(square));
				break;
			case PieceType::bishop:
				score -= GetBitCount(chess->GetBishopAttacks(square, occupancy));
				break;
			case PieceType::rook:
				score -= GetBitCount(chess->GetRookAttacks(square, occupancy));
				break;
			case PieceType::queen:
				score -= GetBitCount(chess->GetBishopAttacks(square, occupancy));
				score -= GetBitCount(chess->GetRookAttacks(square, occupancy));
				break;
			}

			PopBit(pieces, square);
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
