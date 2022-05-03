#include <limits>
#include <algorithm>
#include <iostream>
#include "ChessPlayer.h"

void InitEvaluationMasks();

void ChessPlayer::setupPlayers(ChessPlayer** playerWhite, ChessPlayer** playerBlack, Chess& chess)
{
	*playerBlack = new ChessPlayer(chess, false);
	(*playerBlack)->SetAI(false, 6);

	*playerWhite = new ChessPlayer(chess, true);
	(*playerWhite)->SetAI(false, 6);

	InitEvaluationMasks();
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

	//score += PositionalScores[piece][endPosition] - PositionalScores[piece][startPosition];
	score += (chess.GetAttackCount(chess.IsWhiteTurn(), endPosition) - chess.GetAttackCount(!chess.IsWhiteTurn(), endPosition)) * 10;
	score += (chess.GetAttackCount(!chess.IsWhiteTurn(), startPosition) - chess.GetAttackCount(chess.IsWhiteTurn(), startPosition)) * 10;
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

const int isolatedPawnPenalty = 10;
const int doubledPawnPenalty = 10;
const int passedPawnBonus[8]{ 0, 5, 10, 20, 35, 60, 100, 200 };

constexpr uint64_t files[8]
{
	0x0101010101010101,
	0x0202020202020202,
	0x0404040404040404,
	0x0808080808080808,
	0x1010101010101010,
	0x2020202020202020,
	0x4040404040404040,
	0x8080808080808080,
};

constexpr uint64_t ranks[8]
{
	0x00000000000000ff,
	0x000000000000ff00,
	0x0000000000ff0000,
	0x00000000ff000000,
	0x000000ff00000000,
	0x0000ff0000000000,
	0x00ff000000000000,
	0xff00000000000000,
};

constexpr uint64_t isolatedPawnFiles[8]
{
	0x0202020202020202,
	0x0505050505050505,
	0x0a0a0a0a0a0a0a0a,
	0x1414141414141414,
	0x2828282828282828,
	0x5050505050505050,
	0xa0a0a0a0a0a0a0a0,
	0x4040404040404040,
};


constexpr uint64_t passedPawnMasks[2][64]
{
	0x303030303030300,
	0x707070707070700,
	0xe0e0e0e0e0e0e00,
	0x1c1c1c1c1c1c1c00,
	0x3838383838383800,
	0x7070707070707000,
	0xe0e0e0e0e0e0e000,
	0xc0c0c0c0c0c0c000,
	0x303030303030000,
	0x707070707070000,
	0xe0e0e0e0e0e0000,
	0x1c1c1c1c1c1c0000,
	0x3838383838380000,
	0x7070707070700000,
	0xe0e0e0e0e0e00000,
	0xc0c0c0c0c0c00000,
	0x303030303000000,
	0x707070707000000,
	0xe0e0e0e0e000000,
	0x1c1c1c1c1c000000,
	0x3838383838000000,
	0x7070707070000000,
	0xe0e0e0e0e0000000,
	0xc0c0c0c0c0000000,
	0x303030300000000,
	0x707070700000000,
	0xe0e0e0e00000000,
	0x1c1c1c1c00000000,
	0x3838383800000000,
	0x7070707000000000,
	0xe0e0e0e000000000,
	0xc0c0c0c000000000,
	0x303030000000000,
	0x707070000000000,
	0xe0e0e0000000000,
	0x1c1c1c0000000000,
	0x3838380000000000,
	0x7070700000000000,
	0xe0e0e00000000000,
	0xc0c0c00000000000,
	0x303000000000000,
	0x707000000000000,
	0xe0e000000000000,
	0x1c1c000000000000,
	0x3838000000000000,
	0x7070000000000000,
	0xe0e0000000000000,
	0xc0c0000000000000,
	0x300000000000000,
	0x700000000000000,
	0xe00000000000000,
	0x1c00000000000000,
	0x3800000000000000,
	0x7000000000000000,
	0xe000000000000000,
	0xc000000000000000,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x0,
	0x3,
	0x7,
	0xe,
	0x1c,
	0x38,
	0x70,
	0xe0,
	0xc0,
	0x303,
	0x707,
	0xe0e,
	0x1c1c,
	0x3838,
	0x7070,
	0xe0e0,
	0xc0c0,
	0x30303,
	0x70707,
	0xe0e0e,
	0x1c1c1c,
	0x383838,
	0x707070,
	0xe0e0e0,
	0xc0c0c0,
	0x3030303,
	0x7070707,
	0xe0e0e0e,
	0x1c1c1c1c,
	0x38383838,
	0x70707070,
	0xe0e0e0e0,
	0xc0c0c0c0,
	0x303030303,
	0x707070707,
	0xe0e0e0e0e,
	0x1c1c1c1c1c,
	0x3838383838,
	0x7070707070,
	0xe0e0e0e0e0,
	0xc0c0c0c0c0,
	0x30303030303,
	0x70707070707,
	0xe0e0e0e0e0e,
	0x1c1c1c1c1c1c,
	0x383838383838,
	0x707070707070,
	0xe0e0e0e0e0e0,
	0xc0c0c0c0c0c0,
	0x3030303030303,
	0x7070707070707,
	0xe0e0e0e0e0e0e,
	0x1c1c1c1c1c1c1c,
	0x38383838383838,
	0x70707070707070,
	0xe0e0e0e0e0e0e0,
	0xc0c0c0c0c0c0c0,
};



inline int ChessPlayer::EvaluateMaterial(const GameState& state, bool isWhite)
{
	int mat = 0;

	for (int i = 0; i < 6; ++i)
	{
		mat += GetBitCount(state.pieces[i] & state.occupancy[isWhite]) * pieceScores[i];
	}

	return mat;
}

inline int ChessPlayer::EvaluatePosition(const GameState& state, bool isWhite, bool endGame)
{
	uint64_t pieces = 0;
	int positional = 0;

	for (int i = 0; i < 5; i++)
	{
		pieces = state.pieces[i] & state.occupancy[isWhite];

		while (pieces)
		{
			uint32_t square = GetLeastIndex(pieces);
			positional += PositionalScores[i][square];
			PopBit(pieces, square);
		}
	}

	//King positional eval:
	pieces = state.pieces[(int)PieceType::king - 1] & state.occupancy[1];
	while (pieces)
	{
		uint32_t square = GetLeastIndex(pieces);
		positional += PositionalScores[5 + endGame][square];
		PopBit(pieces, square);
	}

	return positional;
}



inline int ChessPlayer::EvaluatePawnStructure(const GameState& state, bool isWhite)
{
	int structure = 0;
	const uint64_t pawns = state.pieces[(int)PieceType::pawn - 1] & state.occupancy[isWhite];
	const uint64_t opponentPawns = state.pieces[(int)PieceType::pawn - 1] & state.occupancy[!isWhite];
	uint64_t pieces = pawns;

	while (pieces)
	{
		uint32_t square = GetLeastIndex(pieces);
		const int32_t file = ToFile(square);

		if (!(passedPawnMasks[isWhite][square] & opponentPawns))
		{
			structure += passedPawnBonus[isWhite ? 7 - ToRank(square) : ToRank(square)];
		}

		if (!(pawns & isolatedPawnFiles[file]))
		{
			structure -= isolatedPawnPenalty;
		}

		structure -= (GetBitCount(files[file] & pawns)-1) * doubledPawnPenalty;
		
		PopBit(pieces, square);
	}

	return structure;
}

inline int ChessPlayer::EvaluateOpenFiles(const GameState& state, bool isWhite)
{
	return 0;
}

const int protectionScore = 5;
const int threatScore = 20;
const int attackWeight[7]{ 0, 50, 75, 88, 94, 97, 99 };

inline int ChessPlayer::EvaluateKingSafety(const Chess& chess, const GameState& state, bool isWhite)
{
	const int32_t kingSquare = GetLeastIndex(state.pieces[(int)PieceType::king - 1] & state.occupancy[isWhite]);
	uint64_t kingRegion = chess.GetKingAttacks(kingSquare);

	int safety = 0;

	//Evaluate protection:
	safety += GetBitCount(kingRegion & state.occupancy[isWhite]) * protectionScore;

	while (kingRegion)
	{
		int32_t square = GetLeastIndex(kingRegion);
		safety -= chess.GetAttackCount(!isWhite, square) * threatScore;
		PopBit(kingRegion, square);
	}

	return safety;
}

void InitEvaluationMasks()
{

	/*
	std::wcout << L"White:\n";

	//White:
	for (int x = 0; x < 8; ++x)
	{
		//Set ranks.
		uint64_t f = files[x];

		if (x > 0)
		{
			f |= files[x - 1];
		}

		if (x < 7)
		{
			f |= files[x + 1];
		}

		for (int y = 0; y < 8; ++y)
		{
			uint64_t r{ 0 };
			for (int i = y; i < 8; ++i)
			{
				r |= ranks[i];
			}

			passedPawnMasks[1][ToIndex(x, y)] = f & ~r;
		}
	}

	std::wcout << L"Black:\n";

	//black:
	for (int x = 0; x < 8; ++x)
	{
		//Set ranks.
		uint64_t f = files[x];

		if (x > 0)
		{
			f |= files[x - 1];
		}

		if (x < 7)
		{
			f |= files[x + 1];
		}

		for (int y = 0; y < 8; ++y)
		{
			uint64_t r{ 0 };
			for (int i = y; i >= 0; --i)
			{
				r |= ranks[i];
			}

			passedPawnMasks[0][ToIndex(x, y)] = f & ~r;
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		for (int square = 0; square < 64; ++square)
		{
			std::wcout << L"0x" << std::hex << passedPawnMasks[i][square] << L",\n";
		}
	}
	*/
	
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

	const int whiteMaterial = EvaluateMaterial(state, true);
	const int blackMaterial = EvaluateMaterial(state, false);
	int eval = whiteMaterial - blackMaterial;

	eval += EvaluatePosition(state, true, blackMaterial <= 1500);
	eval += EvaluatePawnStructure(state, true);
	eval += EvaluateKingSafety(chess, state, true);

	eval -= EvaluatePosition(state, false, blackMaterial <= 1500);
	eval -= EvaluatePawnStructure(state, false);
	eval -= EvaluateKingSafety(chess, state, true);
	
	return eval * (isWhite ? 1 : -1);
}

/*
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

	*/

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

	if (GetPromoteType(move) != PieceType::none)
	{
		return true;
	}

	//Ensuring that the move is made somewhere with sufficient defencers. 
	const int32_t endPos = GetEndPosition(move);
	if (chess.GetAttackCount(chess.IsWhiteTurn(),endPos) >= chess.GetAttackCount(!chess.IsWhiteTurn(), endPos))
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



