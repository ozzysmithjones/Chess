#include <iostream>
#include <algorithm>
#include <windows.h>   // WinApi header
#include "Chess.h"
#include "Zobrist.h"

#include <fcntl.h>
#include <io.h>
#include <string>



uint64_t Chess::pawnAttacks[2][64] = { 0 };
uint64_t Chess::knightAttacks[64] = { 0 };
uint64_t Chess::kingAttacks[64] = { 0 };
uint64_t Chess::bishopAttacks[64] = { 0 };
uint64_t Chess::rookAttacks[64] = { 0 };
uint64_t Chess::bishopBlockedAttacks[64][512] = { 0 };
uint64_t Chess::rookBlockedAttacks[64][4096] = { 0 };
//ThreeFoldTable Chess::threeFoldTable;


/****************************************************************************\
  ==========================================================================

					Random Number Generation (needed for magic numbers)

  ==========================================================================
\****************************************************************************/


constexpr unsigned int seed = 1804289383;


uint32_t GetRandomU32()
{
	static uint32_t randomState = seed;

	randomState ^= randomState << 13;
	randomState ^= randomState >> 17;
	randomState ^= randomState << 5;

	return randomState;
}

uint64_t GetRandomU64()
{
	uint64_t num1, num2, num3, num4;

	num1 = (uint64_t)(GetRandomU32()) & 0xFFFF;
	num2 = (uint64_t)(GetRandomU32()) & 0xFFFF;
	num3 = (uint64_t)(GetRandomU32()) & 0xFFFF;
	num4 = (uint64_t)(GetRandomU32()) & 0xFFFF;

	return num1 | (num2 << 16) | (num3 << 32) | (num4 << 48);
}

uint64_t CalculateMagicNumberCandidate()
{
	return GetRandomU64() & GetRandomU64() & GetRandomU64();
}



/****************************************************************************\
  ==========================================================================

									Chess

  ==========================================================================
\****************************************************************************/


Chess::Chess()
{
	_setmode(_fileno(stdout), _O_WTEXT);

	zobrist = new Zobrist();
	SetUpBoard();
	InitSliderAttacks(true);
	InitSliderAttacks(false);

	for (int square = 0; square < 64; square++)
	{
		pawnAttacks[1][square] = CalculatePawnAttacks(true, square);
		pawnAttacks[0][square] = CalculatePawnAttacks(false, square);
		knightAttacks[square] = CalculateKnightAttacks(square);
		kingAttacks[square] = CalculateKingAttacks(square);
	}

	irreversables.push(0);
}

Chess::~Chess()
{
	delete zobrist;
}

bool Chess::IsWhiteTurn() const
{
	return isWhiteTurn;
}

const GameState& Chess::GetGameState() const
{
	return log.top();
}

void Chess::SetGameState(const GameState& gameState)
{
	log.top() = gameState;
}


#define UNICODE_PIECES 1

void Chess::PrintBoard(std::vector<Move>& moves) const
{
	if (moves.empty())
	{
		PrintBoard();
		return;
	}

	const GameState& game = log.top();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	constexpr WORD whiteBackground = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
	constexpr WORD blackBackground = 0;
	constexpr WORD whitePiece = FOREGROUND_RED | FOREGROUND_GREEN;
	constexpr WORD blackPiece = FOREGROUND_RED;
	constexpr WORD whiteForeground = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

	for (unsigned y = 0; y < 8u; y++)
	{
		std::wcout << (8 - y) << L": ";

		for (unsigned x = 0; x < 8u; x++)
		{
			const unsigned square = (y << 3) | x;

			const bool isWhiteSquare = !(bool)((x + y) & 1u);
			const bool isWhitePiece = game.occupancy[1] & (1ull << square);
			const bool isBlackPiece = game.occupancy[0] & (1ull << square);
			const PieceType pieceType = GetPieceTypeAt(square);

			const WORD squareColor = isWhiteSquare ? whiteBackground : blackBackground;
			const WORD pieceColor = isWhitePiece ? whitePiece : isBlackPiece ? blackPiece : FOREGROUND_RED;
			SetConsoleTextAttribute(handle, squareColor | pieceColor);

			if (std::any_of(moves.begin(), moves.end(), [square](const Move& move) { return square == GetEndPosition(move); }))
			{
				std::wcout << (pieceType != PieceType::none ? L"X" : L".") << L" ";
			}
			else {

#if UNICODE_PIECES
				std::wcout << unicodePieces[(int)pieceType] << L" ";
#else
				std::wcout << ASCIIPieces[(int)pieceType];
#endif
			}

			SetConsoleTextAttribute(handle, whiteForeground);

		}

		std::wcout << L"\n";
	}

	std::wcout << L"   A B C D  E F G H\n";
	std::wcout << game.zobristKey << L" - Zobrist Key" << L"\n";

	if (game.inCheck)
	{
		std::wcout << L"Check!\n";
	}
}

void Chess::PrintBoard() const
{
	const GameState& game = log.top();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);

	constexpr WORD whiteBackground = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
	constexpr WORD blackBackground = 0;
	constexpr WORD whitePiece = FOREGROUND_RED | FOREGROUND_GREEN;
	constexpr WORD blackPiece = FOREGROUND_RED;
	constexpr WORD whiteForeground = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

	for (unsigned y = 0; y < 8u; y++)
	{
		std::wcout << (8 - y) << L": ";

		for (unsigned x = 0; x < 8u; x++)
		{
			const unsigned square = (y << 3) | x;

			const bool isWhiteSquare = !(bool)((x + y) & 1u);
			const bool isWhitePiece = game.occupancy[1] & (1ull << square);
			const bool isBlackPiece = game.occupancy[0] & (1ull << square);
			const PieceType pieceType = GetPieceTypeAt(square);

			const WORD squareColor = isWhiteSquare ? whiteBackground : blackBackground;
			const WORD pieceColor = isWhitePiece ? whitePiece : isBlackPiece ? blackPiece : whiteForeground;
			SetConsoleTextAttribute(handle, squareColor | pieceColor);

#if UNICODE_PIECES
			std::wcout << unicodePieces[(int)pieceType] << " ";
#else
			std::wcout << ASCIIPieces[(int)pieceType];
#endif

			SetConsoleTextAttribute(handle, whiteForeground);

		}


		std::wcout << L"\n";
	}

	std::wcout << L"   A B C D  E F G H\n";
	std::wcout << game.zobristKey << L" - Zobrist Key" << L"\n";
	if (game.inCheck)
	{
		std::wcout << L"Check!\n";
	}
}

void Chess::CalculateLegalMoves(std::vector<Move>& moves)
{
	CalculateMoves(moves);

	int playerkingSquare = GetKingSquare(isWhiteTurn);
	int opponentKingSquare = GetKingSquare(!isWhiteTurn);

	for (std::size_t i = 0; i < moves.size();)
	{
		MakeMove(moves[i]);

		int kingSquare = GetPieceType(moves[i]) != PieceType::king ? playerkingSquare : GetEndPosition(moves[i]);

		//Not allowed to put yourself in check
		if (IsSquareAttacked(kingSquare, !isWhiteTurn))
		{
			moves[i] = moves.back();
			moves.pop_back();
			UndoMove();
			continue;
		}

		//use this opportunity to check if the opponent is in check from the move
		if (IsSquareAttacked(opponentKingSquare, isWhiteTurn))
		{
			moves[i] = SetCheckMove(moves[i]);
		}

		UndoMove();
		++i;
	}

}


void Chess::CalculateMoves(std::vector<Move>& moves) const
{
	const auto& state = log.top();
	const uint64_t bothOccupancy = state.occupancy[0] | state.occupancy[1];

	uint64_t pawn = state.pieces[(unsigned)PieceType::pawn - 1] & state.occupancy[isWhiteTurn];
	CalculatePawnMoves(pawn, state.occupancy, moves, isWhiteTurn);

	for (uint64_t knight = (state.pieces[(unsigned)PieceType::knight - 1] & state.occupancy[isWhiteTurn]); knight != 0;)
	{
		const unsigned square = GetLeastIndex(knight);
		PopBit(knight, square);
		const uint64_t b = GetKnightAttacks(square) & ~state.occupancy[isWhiteTurn];
		BitBoardToMoves(b, moves, PieceType::knight, square);
	}

	for (uint64_t bishop = (state.pieces[(unsigned)PieceType::bishop - 1] & state.occupancy[isWhiteTurn]); bishop != 0;)
	{
		const unsigned square = GetLeastIndex(bishop);
		PopBit(bishop, square);
		const uint64_t b = GetBishopAttacks(square, bothOccupancy) & ~state.occupancy[isWhiteTurn];
		BitBoardToMoves(b, moves, PieceType::bishop, square);
	}

	for (uint64_t rook = (state.pieces[(unsigned)PieceType::rook - 1] & state.occupancy[isWhiteTurn]); rook != 0;)
	{
		const unsigned square = GetLeastIndex(rook);
		PopBit(rook, square);
		const uint64_t b = GetRookAttacks(square, bothOccupancy) & ~state.occupancy[isWhiteTurn];
		BitBoardToMoves(b, moves, PieceType::rook, square);
	}

	for (uint64_t queen = (state.pieces[(unsigned)PieceType::queen - 1] & state.occupancy[isWhiteTurn]); queen != 0;)
	{
		const unsigned square = GetLeastIndex(queen);
		PopBit(queen, square);
		const uint64_t b = (GetRookAttacks(square, bothOccupancy) | GetBishopAttacks(square, bothOccupancy)) & ~state.occupancy[isWhiteTurn];
		BitBoardToMoves(b, moves, PieceType::queen, square);
	}

	for (uint64_t king = (state.pieces[(unsigned)PieceType::king - 1] & state.occupancy[isWhiteTurn]); king != 0;)
	{
		const unsigned square = GetLeastIndex(king);
		PopBit(king, square);
		const uint64_t b = GetKingAttacks(square) & ~state.occupancy[isWhiteTurn];
		BitBoardToMoves(b, moves, PieceType::king, square);
	}

	if (!state.inCheck)
	{
		//Castling moves:
		const uint32_t castling = ~state.castlingRights;
		const uint64_t rook = state.pieces[(unsigned)PieceType::rook - 1] & state.occupancy[isWhiteTurn];

		if (isWhiteTurn)
		{
			if (!(bothOccupancy & wkCastleAttacked) && (castling & 1u) && !IsSquareAttacked(f1, isWhiteTurn) && !IsSquareAttacked(g1, isWhiteTurn) && (rook & (1ull << h1)))
			{
				moves.emplace_back(CreateMove(e1, g1, MoveType::castleKing, PieceType::king));
			}

			if (!(bothOccupancy & wqCastleAttacked) && (castling & 2u) && !IsSquareAttacked(d1, isWhiteTurn) && !IsSquareAttacked(c1, isWhiteTurn) && (rook & (1ull << a1)))
			{
				moves.emplace_back(CreateMove(e1, c1, MoveType::castleQueen, PieceType::king));
			}
		}
		else
		{
			if (!(bothOccupancy & bkCastleAttacked) && (castling & 4u) && !IsSquareAttacked(f8, isWhiteTurn) && !IsSquareAttacked(g8, isWhiteTurn) && (rook & (1ull << h8)))
			{
				moves.emplace_back(CreateMove(e8, g8, MoveType::castleKing, PieceType::king));
			}

			if (!(bothOccupancy & bqCastleAttacked) && (castling & 8u) && !IsSquareAttacked(d8, isWhiteTurn) && !IsSquareAttacked(c8, isWhiteTurn) && (rook & (1ull << a8)))
			{
				moves.emplace_back(CreateMove(e8, c8, MoveType::castleQueen, PieceType::king));
			}
		}
	}
}

void Chess::MakeNullMove()
{
	//push a new game state on the log.
	log.push(log.top());
	GameState& state = log.top();

	if (state.enpassantSquare != NO_ENPASSANT)
	{
		state.zobristKey ^= zobrist->GetEnpassantSeed(state.enpassantSquare >> 3);
	}

	state.enpassantSquare = NO_ENPASSANT;
	state.inCheck = false;

	//shift to opposite color
	state.zobristKey ^= zobrist->GetTurnSeed();
	isWhiteTurn = !isWhiteTurn;

	//store history for repetitions.
	++repetitionIndex;
	repetitionTable[repetitionIndex] = state.zobristKey;
}

void Chess::MakeMove(const Move move)
{
	//push a new game state on the log.
	log.push(log.top());
	GameState& state = log.top();
	state.move = move;

	if (state.enpassantSquare != NO_ENPASSANT)
	{
		state.zobristKey ^= zobrist->GetEnpassantSeed(state.enpassantSquare >> 3);
	}

	state.enpassantSquare = NO_ENPASSANT;
	state.inCheck = GetIsCheck(move);

	unsigned int startPosition = GetStartPosition(move);
	unsigned int endPosition = GetEndPosition(move);
	PieceType pieceType = GetPieceType(move);

	//decriment attack counts  for move ordering & king safety
	uint64_t attacks = GetPieceAttacks(pieceType, startPosition, state.occupancy[0] | state.occupancy[1], isWhiteTurn);;
	while (attacks)
	{
		int32_t square = GetLeastIndex(attacks);
		--attackCounts[isWhiteTurn][square];
		PopBit(attacks, square);
	}

	//Capture anything on that square.
	PieceType captureType = GetCaptureType(move);
	if (captureType != PieceType::none)
	{
		PopBit(state.pieces[(unsigned int)captureType - 1], endPosition);
		PopBit(state.occupancy[!isWhiteTurn], endPosition);

		state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)captureType - 1, endPosition, !isWhiteTurn);
	}

	//Move the piece:
	PopBit(state.pieces[(unsigned int)pieceType - 1], startPosition);
	SetBit(state.pieces[(unsigned int)pieceType - 1], endPosition);

	PopBit(state.occupancy[isWhiteTurn], startPosition);
	SetBit(state.occupancy[isWhiteTurn], endPosition);

	state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)pieceType - 1, startPosition, isWhiteTurn);
	state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)pieceType - 1, endPosition, isWhiteTurn);

	//Handle rook disallowing castling:
	if (pieceType == PieceType::rook)
	{
		if (isWhiteTurn)
		{
			if (!(state.castlingRights & 1u) && startPosition == h1) // castle king no longer allowed
			{
				state.castlingRights |= 1u;
				state.zobristKey ^= zobrist->GetCastleSeed(true, true);
			}
			else if (!(state.castlingRights & 2u) && startPosition == a1) // castle queen no longer allowed
			{
				state.castlingRights |= 2u;
				state.zobristKey ^= zobrist->GetCastleSeed(false, true);
			}
		}
		else
		{
			if (!(state.castlingRights & 4u) && startPosition == h8) // castle king no longer allowed
			{
				state.castlingRights |= 4u;
				state.zobristKey ^= zobrist->GetCastleSeed(true, false);
			}
			else if (!(state.castlingRights & 8u) && startPosition == a8) // castle queen no longer allowed
			{
				state.castlingRights |= 8u;
				state.zobristKey ^= zobrist->GetCastleSeed(false, false);
			}
		}
	}
	else if (pieceType == PieceType::king)
	{
		if (!(state.castlingRights & 1u))
		{
			state.zobristKey ^= zobrist->GetCastleSeed(true, true);
		}

		if (!(state.castlingRights & 2u))
		{
			state.zobristKey ^= zobrist->GetCastleSeed(false, true);
		}

		if (!(state.castlingRights & 4u))
		{
			state.zobristKey ^= zobrist->GetCastleSeed(true, false);
		}

		if (!(state.castlingRights & 8u))
		{
			state.zobristKey ^= zobrist->GetCastleSeed(false, false);
		}

		state.castlingRights = 15u;
	}


	//Handle special move rules.
	switch (GetMoveType(move))
	{
	case MoveType::normal:
		break;

	case MoveType::advancedPawn:
		state.enpassantSquare = endPosition + (isWhiteTurn ? 8 : -8);
		state.zobristKey ^= zobrist->GetEnpassantSeed(state.enpassantSquare >> 3);
		break;

	case MoveType::enpassantLower:
		PopBit(state.pieces[(unsigned int)PieceType::pawn - 1], startPosition - 1u);
		PopBit(state.occupancy[!isWhiteTurn], startPosition - 1u);

		state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)PieceType::pawn - 1, startPosition - 1, !isWhiteTurn);
		break;

	case MoveType::enpassantHigher:
		PopBit(state.pieces[(unsigned int)PieceType::pawn - 1], startPosition + 1u);
		PopBit(state.occupancy[!isWhiteTurn], startPosition + 1u);

		state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)PieceType::pawn - 1, startPosition + 1, !isWhiteTurn);
		break;

	case MoveType::promote:

		{
			const unsigned promotion = (unsigned int)GetPromoteType(move) - 1;
			PopBit(state.pieces[(unsigned int)PieceType::pawn - 1], endPosition);
			SetBit(state.pieces[promotion], endPosition);

			state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)PieceType::pawn - 1, endPosition, isWhiteTurn);
			state.zobristKey ^= zobrist->GetPieceSeed(promotion, endPosition, isWhiteTurn);
		}

		break;

	case MoveType::castleKing:

		{
			uint32_t startRookSquare;
			uint32_t endRookSquare;

			if (isWhiteTurn)
			{
				startRookSquare = h1;
				endRookSquare = f1;
			}
			else
			{
				startRookSquare = h8;
				endRookSquare = f8;
			}

			PopBit(state.pieces[(unsigned int)PieceType::rook - 1], startRookSquare);
			SetBit(state.pieces[(unsigned int)PieceType::rook - 1], endRookSquare);
			PopBit(state.occupancy[isWhiteTurn], startRookSquare);
			SetBit(state.occupancy[isWhiteTurn], endRookSquare);

			state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)PieceType::rook - 1, startRookSquare, isWhiteTurn);
			state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)PieceType::rook - 1, endRookSquare, isWhiteTurn);
		}

		break;
	case MoveType::castleQueen:

		{
			uint32_t startRookSquare;
			uint32_t endRookSquare;

			if (isWhiteTurn)
			{
				startRookSquare = a1;
				endRookSquare = d1;
			}
			else
			{
				startRookSquare = a8;
				endRookSquare = d8;
			}

			PopBit(state.pieces[(unsigned int)PieceType::rook - 1], startRookSquare);
			SetBit(state.pieces[(unsigned int)PieceType::rook - 1], endRookSquare);
			PopBit(state.occupancy[isWhiteTurn], startRookSquare);
			SetBit(state.occupancy[isWhiteTurn], endRookSquare);

			state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)PieceType::rook - 1, startRookSquare, isWhiteTurn);
			state.zobristKey ^= zobrist->GetPieceSeed((unsigned int)PieceType::rook - 1, endRookSquare, isWhiteTurn);
		}

		break;
	}



	//add attack counts  for move ordering & king safety
	attacks = GetPieceAttacks(pieceType, endPosition, state.occupancy[0] | state.occupancy[1], isWhiteTurn);;
	while (attacks)
	{
		int32_t square = GetLeastIndex(attacks);
		++attackCounts[isWhiteTurn][square];
		PopBit(attacks, square);
	}

	//switch turn:
	state.zobristKey ^= zobrist->GetTurnSeed();
	isWhiteTurn = !isWhiteTurn;
	
	//store history for repetitions:
	++repetitionIndex;
	repetitionTable[repetitionIndex] = state.zobristKey;
	
	if (GetIsIrreversible(move))
	{
		irreversables.push(repetitionIndex);
	}
}

void Chess::UndoMove()
{
	Move move = log.top().move;
	if (move)
	{
		//decriment attack counts  for move ordering & king safety
		uint64_t attacks = GetPieceAttacks(GetPieceType(move), GetEndPosition(move), log.top().occupancy[0] | log.top().occupancy[1], !isWhiteTurn);;
		while (attacks)
		{
			int32_t square = GetLeastIndex(attacks);
			--attackCounts[!isWhiteTurn][square];
			PopBit(attacks, square);
		}
	}


	isWhiteTurn = !isWhiteTurn;
	log.pop();

	if (move)
	{
		//decriment attack counts  for move ordering & king safety
		uint64_t attacks = GetPieceAttacks(GetPieceType(move), GetStartPosition(move), log.top().occupancy[0] | log.top().occupancy[1], isWhiteTurn);;
		while (attacks)
		{
			int32_t square = GetLeastIndex(attacks);
			++attackCounts[isWhiteTurn][square];
			PopBit(attacks, square);
		}
	}

	if (irreversables.top() == repetitionIndex)
	{
		irreversables.pop();
	}

	--repetitionIndex;
}

bool Chess::IsCheck() const
{
	return log.top().inCheck;
}

bool Chess::IsDraw() const
{

	if ((repetitionIndex - irreversables.top()) >= 50) // 50 move rule.
	{
		return true;
	}

	for (std::size_t i = irreversables.top(); i < repetitionIndex; ++i)
	{
		if (repetitionTable[i] == repetitionTable[repetitionIndex])
		{
			return true;
		}
	}
	
	return false;
}

int Chess::GetAttackCount(bool isWhite, int square) const
{
	return attackCounts[isWhite][square];
}

uint64_t Chess::GetPieceAttacks(PieceType pieceType, int square, uint64_t occupancy, bool isWhite)
{
	switch (pieceType)
	{
	case PieceType::none:
		return 0ull;
	case PieceType::pawn:
		return GetPawnAttacks(isWhite, square);
	case PieceType::knight:
		return GetKnightAttacks(square);
	case PieceType::bishop:
		return GetBishopAttacks(square, occupancy);
	case PieceType::rook:
		return GetRookAttacks(square, occupancy);
	case PieceType::queen:
		return GetBishopAttacks(square, occupancy) | GetRookAttacks(square, occupancy);
	case PieceType::king:
		return GetKingAttacks(square);
	}

	return 0ull;
}

uint64_t Chess::GetPawnAttacks(bool isWhite, int square) const
{
	return pawnAttacks[isWhite][square];
}

uint64_t Chess::GetKnightAttacks(int square) const
{
	return knightAttacks[square];
}

uint64_t Chess::GetBishopAttacks(int square, uint64_t occupancy) const
{
	occupancy &= bishopAttacks[square];
	occupancy *= bishopMagicNumbers[square];
	occupancy >>= (64ull - bishopAttackCountBySquare[square]);

	return bishopBlockedAttacks[square][occupancy];
}

uint64_t Chess::GetRookAttacks(int square, uint64_t occupancy) const
{
	occupancy &= rookAttacks[square];
	occupancy *= rookMagicNumbers[square];
	occupancy >>= (64ull - rookAttackCountBySquare[square]);

	return rookBlockedAttacks[square][occupancy];
}

uint64_t Chess::GetKingAttacks(int square) const
{
	return kingAttacks[square];
}

void Chess::SetUpBoard()
{
	log.emplace();
	GameState& game = log.top();

	isWhiteTurn = true;
	game.occupancy[0] = 0x000000000000ffffULL;
	game.occupancy[1] = 0xffff000000000000ULL;

	game.pieces[(int)PieceType::pawn - 1]    = 0x00ff00000000ff00ULL;
	game.pieces[(int)PieceType::knight - 1]  = 0x4200000000000042ULL;
	game.pieces[(int)PieceType::bishop - 1]  = 0x2400000000000024ULL;
	game.pieces[(int)PieceType::rook - 1]    = 0x8100000000000081ULL;
	game.pieces[(int)PieceType::queen - 1]   = 0x800000000000008ULL;
	game.pieces[(int)PieceType::king - 1]    = 0x1000000000000010ULL;

	game.zobristKey = zobrist->CalculateZobristKey(true, game);
	uint64_t pieces;
	uint64_t attacks;

	for (int i = 0; i < 6; ++i)
	{
		pieces = game.pieces[i] & game.occupancy[1];

		while (pieces)
		{
			int32_t square = GetLeastIndex(pieces);

			attacks = GetPieceAttacks((PieceType)(i + 1), square, game.occupancy[0] | game.occupancy[1], true);
			while (attacks)
			{
				int32_t attackSquare = GetLeastIndex(attacks);
				++attackCounts[1][attackSquare];
				PopBit(attacks, attackSquare);
			}

			PopBit(pieces, square);
		}


		pieces = game.pieces[i] & game.occupancy[0];

		while (pieces)
		{
			int32_t square = GetLeastIndex(pieces);

			attacks = GetPieceAttacks((PieceType)(i + 1), square, game.occupancy[0] | game.occupancy[1], false);
			while (attacks)
			{
				int32_t attackSquare = GetLeastIndex(attacks);
				++attackCounts[0][attackSquare];
				PopBit(attacks, attackSquare);
			}

			PopBit(pieces, square);
		}
	}
	
}

PieceType Chess::GetPieceTypeAt(int square) const
{
	const uint64_t b = (1ull << square);
	const auto& pieces = log.top().pieces;

	for (int i = 0; i < 6; i++)
	{
		if (pieces[i] & b)
			return (PieceType)(i + 1);
	}

	return PieceType::none;
}

PieceType Chess::GetPieceAt(int square, bool& isWhite) const
{

	const uint64_t b = (1ull << square);
	const auto& pieces = log.top().pieces;

	for (int i = 0; i < 6; i++)
	{
		if (pieces[i] & b)
		{
			isWhite = log.top().occupancy[1] & b;
			return (PieceType)(i + 1);
		}
	}

	isWhite = false;
	return PieceType::none;
}



inline int Chess::GetKingSquare(bool isWhite)
{
	GameState& game = log.top();
	return GetLeastIndex(game.pieces[(int)PieceType::king - 1] & game.occupancy[isWhite]);
}

void Chess::AddPotentialPromoteMove(std::vector<Move>& moves, Move move, int rank, int promotionRank) const
{
	if (rank == promotionRank)
	{
		moves.push_back(SetPromoteMove(move, PieceType::knight));
		moves.push_back(SetPromoteMove(move, PieceType::bishop));
		moves.push_back(SetPromoteMove(move, PieceType::rook));
		moves.push_back(SetPromoteMove(move, PieceType::queen));
	}

	moves.push_back(move);
}

bool Chess::IsMoveCheck(int startSquare, int endSquare, PieceType pieceType, bool isWhite)
{
	const GameState& state = log.top();
	uint64_t occupancy = (state.occupancy[0] | state.occupancy[1]);
	PopBit(occupancy, startSquare);
	SetBit(occupancy, endSquare);
	return IsSquareAttackedBy(GetKingSquare(!isWhite), isWhite, pieceType, occupancy);
}

bool Chess::IsMovePin(int startSquare, int endSquare, PieceType pieceType, bool isWhite)
{
	return false;
}


bool Chess::IsSquareAttacked(int square, bool isWhite) const
{
	const uint64_t b = (1ull << square);
	const GameState& state = log.top();
	const uint64_t occupancy = state.occupancy[0] | state.occupancy[1];

	uint64_t potential = GetPawnAttacks(isWhite, square);
	if (state.pieces[(unsigned)PieceType::pawn - 1] & state.occupancy[!isWhite] & potential)
		return true;

	potential = GetKnightAttacks(square);
	if (state.pieces[(unsigned)PieceType::knight - 1] & state.occupancy[!isWhite] & potential)
		return true;

	const uint64_t bishopPotential = GetBishopAttacks(square, occupancy);
	if (state.pieces[(unsigned)PieceType::bishop - 1] & state.occupancy[!isWhite] & bishopPotential)
		return true;

	const uint64_t rookPotential = GetRookAttacks(square, occupancy);
	if (state.pieces[(unsigned)PieceType::rook - 1] & state.occupancy[!isWhite] & rookPotential)
		return true;

	potential = bishopPotential | rookPotential;
	if (state.pieces[(unsigned)PieceType::queen - 1] & state.occupancy[!isWhite] & potential)
		return true;

	potential = GetKingAttacks(square);
	if (state.pieces[(unsigned)PieceType::king - 1] & state.occupancy[!isWhite] & potential)
		return true;

	return false;
}

bool Chess::IsSquareAttackedBy(int square, bool isWhite, PieceType pieceType, uint64_t occupancy)
{
	const uint64_t b = (1ull << square);
	const GameState& state = log.top();
	uint64_t potential{ 0ull };

	switch (pieceType)
	{
	case PieceType::none:
		return false;
	case PieceType::pawn:
		potential = GetPawnAttacks(isWhite, square);
		break;
	case PieceType::knight:
		potential = GetKnightAttacks(square);;
		break;
	case PieceType::bishop:
		potential = GetBishopAttacks(square, occupancy);
		break;
	case PieceType::rook:
		potential = GetRookAttacks(square, occupancy);
		break;
	case PieceType::queen:
		potential = GetRookAttacks(square, occupancy) | GetBishopAttacks(square, occupancy);
		break;
	case PieceType::king:
		potential = GetKingAttacks(square);
		break;
	}

	return state.pieces[(unsigned)pieceType - 1] & state.occupancy[!isWhite] & potential;
}


void Chess::CalculatePawnMoves(uint64_t pawn, const uint64_t* occupancies, std::vector<Move>& moves, const bool isWhite) const
{
	//Figure out forward moves:

	const uint64_t empty = ~(occupancies[0] | occupancies[1]);
	int promoteRank;
	uint64_t forward{ 0 };
	uint64_t advanced{ 0 };

	if (isWhite)
	{
		forward = (pawn >> 8) & empty;
		advanced = ((forward & rank3) >> 8) & empty; // advanced pawn
		promoteRank = 0;
	}
	else
	{
		forward = (pawn << 8) & empty;
		advanced = ((forward & rank6) << 8) & empty; // advanced pawn
		promoteRank = 7;
	}

	//figure out enpassant position:
	const uint64_t enpassant = 1ull << (log.top().enpassantSquare);

	while (pawn)
	{
		const uint32_t square = GetLeastIndex(pawn);
		const uint64_t b(1ull << square);

		//loop through attack moves and add them:
		uint64_t attacks = GetPawnAttacks(isWhite, square) & occupancies[!isWhite];

		while (attacks)
		{
			const uint32_t endSquare = GetLeastIndex(attacks);
			AddPotentialPromoteMove(moves, CreateMove(square, endSquare, MoveType::normal, PieceType::pawn, GetPieceTypeAt(endSquare)), endSquare >> 3, promoteRank);
			PopBit(attacks, endSquare);
		}

		//add enpassant attack (if any) (there could only be one).
		uint64_t enpassantAttacks = GetPawnAttacks(isWhite, square) & enpassant;
		if (enpassantAttacks)
		{
			const uint32_t endSquare = GetLeastIndex(enpassantAttacks);
			const bool higher = (endSquare & 7) > (square & 7);
			moves.emplace_back(CreateMove(square, endSquare, higher ? MoveType::enpassantHigher : MoveType::enpassantLower, PieceType::pawn));
		}

		//loop through forward moves and add them:
		uint64_t forwards;
		if (isWhite)
		{
			forwards = ((b >> 8)) & forward;
		}
		else
		{
			forwards = ((b << 8)) & forward;
		}

		if (forwards)
		{
			const uint32_t endSquare = GetLeastIndex(forwards);
			AddPotentialPromoteMove(moves, CreateMove(square, endSquare, MoveType::normal, PieceType::pawn), endSquare >> 3, promoteRank);
		}

		//loop through advanced forward moves and add them:
		uint64_t advances;
		if (isWhite)
		{
			advances = (b >> 16) & advanced;
		}
		else
		{
			advances = (b << 16) & advanced;
		}

		if (advances)
		{
			const uint32_t endSquare = GetLeastIndex(advances);
			moves.emplace_back(CreateMove(square, endSquare, MoveType::advancedPawn, PieceType::pawn));
		}

		PopBit(pawn, square);
	}
}

void Chess::BitBoardToMoves(const uint64_t bitboard, std::vector<Move>& moves, const PieceType pieceType, const uint32_t originSquare) const
{
	const auto& state = log.top();
	uint64_t quiet = bitboard & ~state.occupancy[!isWhiteTurn];
	uint64_t capture = bitboard & state.occupancy[!isWhiteTurn];

	while (quiet != 0)
	{
		const unsigned endSquare = GetLeastIndex(quiet);
		moves.push_back(CreateMove(originSquare, endSquare, MoveType::normal, pieceType));
		PopBit(quiet, endSquare);
	}

	while (capture != 0)
	{
		const unsigned endSquare = GetLeastIndex(capture);
		moves.push_back(CreateMove(originSquare, endSquare, MoveType::normal, pieceType, GetPieceTypeAt(endSquare)));
		PopBit(capture, endSquare);
	}
}

uint64_t Chess::CalculatePawnAttacks(bool isWhite, int square)
{
	uint64_t attacks = 0ull;
	uint64_t bitboard = 0ull;

	//Place pawn on the bit board.
	SetBit(bitboard, square);

	if (isWhite) // white pawns
	{
		attacks |= (bitboard & notHFile) >> 7;
		attacks |= (bitboard & notAFile) >> 9;
	}
	else
	{
		attacks |= (bitboard & notAFile) << 7;
		attacks |= (bitboard & notHFile) << 9;
	}

	return attacks;
}

uint64_t Chess::CalculateKnightAttacks(int square)
{
	uint64_t attacks = 0ull;
	uint64_t bitboard = 0ull;

	//Place knight on the bit board.
	SetBit(bitboard, square);

	attacks |= (bitboard & notAFile) >> 17;
	attacks |= (bitboard & notHFile) >> 15;
	attacks |= (bitboard & notABFile) >> 10;
	attacks |= (bitboard & notHGFile) >> 6;

	attacks |= (bitboard & notHFile) << 17;
	attacks |= (bitboard & notAFile) << 15;
	attacks |= (bitboard & notHGFile) << 10;
	attacks |= (bitboard & notABFile) << 6;

	return attacks;
}

uint64_t Chess::CalculateKingAttacks(int square)
{
	uint64_t attacks = 0ull;
	uint64_t bitboard = 0ull;

	//Place knight on the bit board.
	SetBit(bitboard, square);

	attacks |= bitboard >> 8;
	attacks |= (bitboard & notAFile) >> 9;
	attacks |= (bitboard & notHFile) >> 7;
	attacks |= (bitboard & notAFile) >> 1;

	attacks |= bitboard << 8;
	attacks |= (bitboard & notHFile) << 9;
	attacks |= (bitboard & notAFile) << 7;
	attacks |= (bitboard & notHFile) << 1;

	return attacks;
}

uint64_t Chess::CalculateBishopAttacks(int square)
{
	uint64_t attacks = 0ull;

	int y, x;
	int startY = (square /8);
	int startX = (square % 8);

	for (y = startY + 1, x = startX + 1; y < 7 && x < 7; y++, x++)
	{
		attacks |= (1ull << ToIndex(x, y));
	}

	for (y = startY - 1, x = startX + 1; y > 0 && x < 7; y--, x++)
	{
		attacks |= (1ull << ToIndex(x, y));
	}

	for (y = startY + 1, x = startX - 1; y < 7 && x > 0; y++, x--)
	{
		attacks |= (1ull << ToIndex(x, y));
	}

	for (y = startY - 1, x = startX - 1; y > 0 && x > 0; y--, x--)
	{
		attacks |= (1ull << ToIndex(x, y));
	}

	return attacks;
}

uint64_t Chess::CalculateRookAttacks(int square)
{
	uint64_t attacks = 0ull;

	int y, x;
	int startY = (square / 8);
	int startX = (square % 8);

	for (y = startY + 1, x = startX; y < 7; y++)
	{
		attacks |= (1ull << ToIndex(x, y));
	}

	for (y = startY - 1, x = startX; y > 0; y--)
	{
		attacks |= (1ull << ToIndex(x, y));
	}

	for (x = startX + 1, y = startY; x < 7; x++)
	{
		attacks |= (1ull << ToIndex(x, y));
	}

	for (x = startX - 1, y = startY; x > 0; x--)
	{
		attacks |= (1ull << ToIndex(x, y));
	}

	return attacks;
}

uint64_t Chess::CalculateBlockedBishopAttacks(int square, uint64_t occupancy)
{
	uint64_t attacks = 0ull;

	int y, x;
	int startY = square / 8;
	int startX = square % 8;

	for (y = startY + 1, x = startX + 1; y <= 7 && x <= 7; y++, x++)
	{
		const uint32_t index = ToIndex(x, y);
		attacks |= (1ull << index);
		if (GetBit(occupancy, index))
			break;
	}

	for (y = startY - 1, x = startX + 1; y >= 0 && x <= 7; y--, x++)
	{
		const uint32_t index = ToIndex(x, y);
		attacks |= (1ull << index);
		if (GetBit(occupancy, index))
			break;
	}

	for (y = startY + 1, x = startX - 1; y <= 7 && x >= 0; y++, x--)
	{
		const uint32_t index = ToIndex(x, y);
		attacks |= (1ull << index);
		if (GetBit(occupancy, index))
			break;
	}

	for (y = startY - 1, x = startX - 1; y >= 0 && x >= 0; y--, x--)
	{
		const uint32_t index = ToIndex(x, y);
		attacks |= (1ull << index);
		if (GetBit(occupancy, index))
			break;
	}

	return attacks;
}

uint64_t Chess::CalculateBlockedRookAttacks(int square, uint64_t occupancy)
{
	uint64_t attacks = 0ull;

	int y, x;
	int startY = square / 8;
	int startX = square % 8;

	for (y = startY + 1, x = startX; y <= 7; y++)
	{
		const uint32_t index = ToIndex(x, y);
		attacks |= (1ull << index);
		if (GetBit(occupancy, index))
			break;
	}

	for (y = startY - 1, x = startX; y >= 0; y--)
	{
		const uint32_t index = ToIndex(x, y);
		attacks |= (1ull << index);
		if (GetBit(occupancy, index))
			break;
	}

	for (x = startX + 1, y = startY; x <= 7; x++)
	{
		const uint32_t index = ToIndex(x, y);
		attacks |= (1ull << index);
		if (GetBit(occupancy, index))
			break;
	}

	for (x = startX - 1, y = startY; x >= 0; x--)
	{
		const uint32_t index = ToIndex(x, y);
		attacks |= (1ull << index);
		if (GetBit(occupancy, index))
			break;
	}

	return attacks;
}

uint64_t Chess::CalculateMagicNumber(int square, int attackCount, bool isBishop)
{
	const auto maxPermutations = 1u << attackCount;

	uint64_t* occupancyPermutations = new uint64_t[maxPermutations];
	uint64_t* attacks = new uint64_t[maxPermutations];
	uint64_t* usedAttacks = new uint64_t[maxPermutations];
	uint64_t attackMask = isBishop ? CalculateBishopAttacks(square) : CalculateRookAttacks(square);

	// loop through all permutations of possible positions, computing the attack set of the sliding piece in each position.
	// To minimise the number of permutations, only the squares along the ray directions are permuted.

	for (unsigned int index = 0; index < maxPermutations; index++)
	{
		occupancyPermutations[index] = CalculatePermutation(index, attackCount, attackMask);
		attacks[index] = isBishop
			? CalculateBlockedBishopAttacks(square, occupancyPermutations[index])
			: CalculateBlockedRookAttacks(square, occupancyPermutations[index]);
	}

	// Search for the first random number that can be used as a magic number. A magic number is one that creates a unique index for every position with a different attack pattern
	// using the following formula (occupancy * magicnumber) >> (64 - maxNumAttackedSquaresInPosition)

	for (int i = 0; i < 100000000; i++)
	{
		uint64_t magicNumber = CalculateMagicNumberCandidate();

		//skip inapproriate numbers:
		if ((GetBitCount((attackMask * magicNumber) & 0xFF00000000000000)) < 6) continue;

		std::fill(usedAttacks, usedAttacks + maxPermutations, 0ull);
		bool fail = false;
		unsigned int index;

		//test magic index:
		for (index = 0; !fail && index < maxPermutations; index++)
		{
			int magicIndex = (int)((occupancyPermutations[index] * magicNumber) >> (64 - attackCount));

			if (usedAttacks[magicIndex] == 0ull)
			{
				usedAttacks[magicIndex] = attacks[index];
			}
			else if (usedAttacks[magicIndex] != attacks[index])
			{
				fail = true;
			}
		}

		if (!fail)
		{
			delete[] occupancyPermutations;
			delete[] attacks;
			delete[] usedAttacks;

			return magicNumber;
		}
	}

	delete[] occupancyPermutations;
	delete[] attacks;
	delete[] usedAttacks;

	std::cout << "Magic number fails!\n";
	return 0ull;
}




void Chess::GenerateMagicNumbers()
{
	
	for (int i = 0; i < 64; i++)
	{
		std::wcout << L"0x" << std::hex << CalculateMagicNumber(i, rookAttackCountBySquare[i], false) << L",\n";
	}
	
	

	std::wcout << "\n\n";

	
	for (int i = 0; i < 64; i++)
	{
		std::wcout << L"0x" << std::hex << CalculateMagicNumber(i, bishopAttackCountBySquare[i], true) << L",\n";
	}
	
}


void Chess::InitSliderAttacks(bool isBishop)
{
	uint64_t* attacks = isBishop ? bishopAttacks : rookAttacks;
	const int* attackCounts = isBishop ? bishopAttackCountBySquare : rookAttackCountBySquare;

	for (int square = 0; square < 64; square++)
	{
		attacks[square] = isBishop ? CalculateBishopAttacks(square) : CalculateRookAttacks(square);
		const uint64_t attack = attacks[square];
		const int attackCount = GetBitCount(attack);
		const int occupancyPermutations = (1ull << attackCount);

		for (int j = 0; j < occupancyPermutations; j++)
		{
			const uint64_t occupancy = CalculatePermutation(j, attackCount, attack);

			if (isBishop)
			{
				const int magicIndex = (int)((occupancy * bishopMagicNumbers[square]) >> (64 - attackCount));
				bishopBlockedAttacks[square][magicIndex] = CalculateBlockedBishopAttacks(square, occupancy);
			}
			else
			{
				const int magicIndex = (int)((occupancy * rookMagicNumbers[square]) >> (64 - attackCount));
				rookBlockedAttacks[square][magicIndex] = CalculateBlockedRookAttacks(square, occupancy);
			}
		}
	}
}
