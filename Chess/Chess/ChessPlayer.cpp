#include <limits>
#include <algorithm>
#include <iostream>
#include "ChessPlayer.h"


void ChessPlayer::setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, Chess& chess)
{
	*playerBlack = new ChessPlayer(chess, false);
	(*playerBlack)->SetAI(false, 8);

	*playerWhite = new ChessPlayer(chess, true);
	(*playerWhite)->SetAI(false, 8);
}

ChessPlayer::ChessPlayer(Chess& chess, bool isWhite)
	: chess(chess), isWhite(isWhite)
{
	transpositionTable = new TranspositionTable();
}

ChessPlayer::~ChessPlayer()
{
	delete transpositionTable;
}

void ChessPlayer::SetAI(bool _random, int _depth)
{
	isAI = true;
	random = _random;
	depth = _depth;
}

constexpr auto windowSize = 50;

// chooseAIMove
// in this method - for an AI chess player - choose a move to make. This is called once per play. 
bool ChessPlayer::chooseAIMove(std::vector<Move>& moves, Move& moveToMake)
{
	if (random)
	{
		moveToMake = moves[rand() % moves.size()];
		return true;
	}


	//clear garbage data for search.
	std::fill(&killerMoves[0][0], &killerMoves[1][MAX_PLY - 1], 0);
	std::fill(&historyMoveScores[0][0], &historyMoveScores[5][63], 0);
	//transpositionTable->Clear();
	moveToPlay = 0;

	// Iterative depening:
	int alpha = -INFINITY + 1;
	int beta = INFINITY - 1;
	int score;

	for (int i = 1; i <= depth;)
	{
		score = NegaMax(alpha, beta, i);
		//std::wcout << L"Depth: " << i << L"Node count = " << nodes << L"\n";

		if (score <= alpha || score >= beta) // aspiration window fell outside the window, so try again with a full width
		{
			//std::wcout << L"Window out of bounds, try again\n";
			alpha = -INFINITY + 1;
			beta = INFINITY - 1;
			continue;
		}

		alpha = score - windowSize;
		beta = score + windowSize;
		++i;
	}

	//Normal:

	//int score = NegaMax(chess, -INFINITY, INFINITY, depth, followingPV);
	//std::wcout << L"Depth: " << depth << L"Score = " << score << L"\n";
	moveToMake = this->moveToPlay;
	return moveToMake != 0;
}



constexpr static int captureTable[6][6]
{
	105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,
};

int ChessPlayer::ScoreMove(const Move move)
{
	const int32_t piece = (int32_t)GetPieceType(move) - 1;
	const int32_t capture = (int32_t)GetCaptureType(move) - 1;

	if (capture >= 0) //Rank captures before anything else.
	{
		return captureTable[piece][capture] + 10000;
	}

	int score = 0;
	if (ply < MAX_PLY)
	{
		if (killerMoves[0][ply] == move)
		{
			return 9000;
		}

		if (killerMoves[1][ply] == move)
		{
			return 8000;
		}

		score += historyMoveScores[(int)GetPieceType(move) - 1][GetEndPosition(move)];
	}

	const int32_t startPosition = GetStartPosition(move);
	const int32_t endPosition = GetEndPosition(move);
	score += PositionalScores[piece][endPosition] - PositionalScores[piece][startPosition];
	score += GetIsCheck(move) ? 100 : 0;
	return score;
}

void ChessPlayer::Sort(std::vector<Move>& moves, Move transpositionMove)
{
	static std::vector<int> scores;
	scores.resize(moves.size());

	if (transpositionMove) // follow principle variation:
	{
		for (int32_t i = 0; i < moves.size(); ++i)
		{
			if (moves[i] == transpositionMove)
			{
				scores[i] = (INFINITY - 2);
			}
			else
			{
				scores[i] = ScoreMove(moves[i]);
			}
		}
	}
	else
	{
		for (int32_t i = 0; i < moves.size(); ++i)
		{
			scores[i] = ScoreMove(moves[i]);
		}
	}

	for (int step = 0; step < moves.size(); step++) {
		int min_idx = step;
		for (int i = step + 1; i < moves.size(); i++) {

			if (scores[i] > scores[min_idx])
				min_idx = i;
		}

		std::swap(moves[min_idx], moves[step]);
		std::swap(scores[min_idx], scores[step]);
	}
}

int ChessPlayer::Evaluate(const GameState& state, bool isWhite)
{
	//king capture(should not happen, but just in case)
	if (!(state.pieces[(int)PieceType::king - 1] & state.occupancy[1]))
	{
		return -INFINITY * (isWhite ? 1 : -1);
	}
	else if (!(state.pieces[(int)PieceType::king - 1] & state.occupancy[0]))
	{
		return INFINITY * (isWhite ? 1 : -1);
	}

	int eval = 0;
	int whiteMaterial = 0, blackMaterial = 0;
	uint64_t pieces;

	for (int i = 0; i < 5; i++)
	{
		//White:
		pieces = state.pieces[i] & state.occupancy[1];
		whiteMaterial += GetBitCount(pieces) * pieceScores[i];

		while (pieces)
		{
			uint32_t square = GetLeastIndex(pieces);
			eval += PositionalScores[i][square];
			PopBit(pieces, square);
		}

		//Black:
		pieces = state.pieces[i] & state.occupancy[0];
		blackMaterial += GetBitCount(pieces) * pieceScores[i];

		while (pieces)
		{
			uint32_t square = GetLeastIndex(pieces);
			eval -= PositionalScores[i][mirror[square]];
			PopBit(pieces, square);
		}
	}

	eval += (whiteMaterial - blackMaterial);

	//King positional eval:
	bool endGame = (blackMaterial <= 1500);
	pieces = state.pieces[(int)PieceType::king - 1] & state.occupancy[1];
	while (pieces)
	{
		uint32_t square = GetLeastIndex(pieces);
		eval += PositionalScores[5 + endGame][square];
		PopBit(pieces, square);
	}

	endGame = (whiteMaterial <= 1500);
	pieces = state.pieces[(int)PieceType::king - 1] & state.occupancy[0];
	while (pieces)
	{
		uint32_t square = GetLeastIndex(pieces);
		eval -= PositionalScores[5 + endGame][mirror[square]];
		PopBit(pieces, square);
	}

	//additional pawn eval
	//eval -= GetIslandCount(state.pieces[(int)PieceType::pawn - 1] & state.occupancy[1]);
	//eval += GetIslandCount(state.pieces[(int)PieceType::pawn - 1] & state.occupancy[0]);

	return eval * (isWhite ? 1 : -1);
}

bool ChessPlayer::IsMovePromising(const Move move)
{
	if (GetIsCheck(move))
	{
		return true;
	}

	if (GetCaptureType(move) != PieceType::none)
	{
		return true;
	}


	if (GetPieceType(move) != PieceType::pawn)
	{
		return true;
	}


	/*
	//responding to a threat made by the opponent:
	if (((1ull << GetEndPosition(move)) & threat) || ((1ull << GetStartPosition(move)) & threat))
	{
		return true;
	}
	*/


	if (GetPromoteType(move) != PieceType::none)
	{
		return true;
	}

	return false;
}

int32_t ChessPlayer::Quiescence(int alpha, int beta)
{
	const int32_t evaluation = Evaluate(chess.GetGameState(), chess.IsWhiteTurn());

	if (evaluation >= beta) // fails high
	{
		return beta;
	}

	if (evaluation > alpha)
	{
		alpha = evaluation;
	}

	std::vector<Move> moves;
	chess.CalculateLegalMoves(moves);
	if (moves.empty())
	{
		return chess.IsCheck() ? (-INFINITY + ply + 2) : 0;
	}

	Sort(moves, 0);

	for (const auto& move : moves)
	{
		if (GetCaptureType(move) == PieceType::none)
		{
			continue;
		}

		++ply;
		chess.MakeMove(move);
		int score = -Quiescence(-beta, -alpha);
		chess.UndoMove();
		--ply;

		if (score >= beta) // fails high
		{
			return beta;
		}

		if (score > alpha)
		{
			alpha = score;
		}
	}

	return alpha; // fails low
}
int32_t ChessPlayer::NegaMax(int alpha, int beta, int depth)
{
	if (chess.IsDraw())
	{
		return 0;
	}

	Move transpositionMove;
	if (auto score = transpositionTable->Get(chess.GetGameState().zobristKey, depth, alpha, beta, transpositionMove))
	{
		if (ply == 0)
		{
			moveToPlay = transpositionMove;
		}

		return score.value();
	}

	if (depth <= 0)
	{
		return Quiescence(alpha, beta);
	}

	int score;

	//NULL move prunning
	if (depth > 3 && !chess.IsCheck() && ply)
	{
		++ply;
		chess.MakeNullMove();
		score = -NegaMax(-beta, -(beta - 1), depth - 3);
		chess.UndoMove();
		--ply;

		if (score >= beta)
		{
			transpositionTable->Set(chess.GetGameState().zobristKey, depth, beta, ScoreType::Beta);
			return beta;
		}
	}

	std::vector<Move> moves;
	chess.CalculateLegalMoves(moves);
	if (moves.empty())
	{
		score = chess.IsCheck() ? (-INFINITY + (ply + 2)) : 0;
		transpositionTable->Set(chess.GetGameState().zobristKey, depth, score, ScoreType::Exact);
		return score;
	}

	if (ply >= MAX_PLY)
	{
		return Evaluate(chess.GetGameState(), chess.IsWhiteTurn());
	}

	Sort(moves, transpositionMove);
	bool pvs = false;
	ScoreType hashScoreType = ScoreType::Alpha;
	Move bestMove = 0;

	for (int32_t i = 0; i < moves.size(); ++i)
	{
		const Move& move = moves[i];

		++ply;
		chess.MakeMove(move);

		if (i >= 4 && depth >= 3 && !IsMovePromising(move)) // do reduced search if move is not promising
		{
			// verify that this move will not improve score with smaller window (alpha +1) and depth
			score = -NegaMax(-(alpha + 1), -alpha, depth - 2);

			//if score improved (failure)
			if (score > alpha && score < beta)
			{
				//do normal search
				score = -NegaMax(-beta, -alpha, depth - 1);
			}
		}
		else if (pvs) // do reduced search if we already found a good move.
		{
			// verify that this move will not improve score with smaller window (alpha +1), but same depth
			score = -NegaMax(-(alpha + 1), -alpha, depth - 1);

			//if score improved (failure)
			if (score > alpha && score < beta)
			{
				//do normal search
				score = -NegaMax(-beta, -alpha, depth - 1);
			}
		}
		else
		{
			//do normal search
			score = -NegaMax(-beta, -alpha, depth - 1);
		}

		chess.UndoMove();
		--ply;

		if (score >= beta) // fails high
		{
			transpositionTable->Set(chess.GetGameState().zobristKey, depth, beta, ScoreType::Beta, move);

			if (GetCaptureType(move) == PieceType::none)
			{
				//add killer move:
				killerMoves[1][ply] = killerMoves[0][ply];
				killerMoves[0][ply] = move;
			}

			if (ply == 0)
			{
				moveToPlay = move;
			}

			return beta;
		}

		if (score > alpha)
		{
			hashScoreType = ScoreType::Exact;
			bestMove = move;

			if (GetCaptureType(move) == PieceType::none)
			{
				//add history move:
				historyMoveScores[(int)GetPieceType(move) - 1][GetEndPosition(move)] += depth * depth;
			}

			alpha = score;
			pvs = true;
		}
	}

	if (ply == 0)
	{
		moveToPlay = bestMove;
	}

	transpositionTable->Set(chess.GetGameState().zobristKey, depth, alpha, hashScoreType, bestMove);
	return alpha; // fails low
}



