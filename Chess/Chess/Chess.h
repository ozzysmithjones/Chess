#pragma once
#include <stdint.h>
#include <stack>
#include <vector>
#include "Piece.h"
#include "Constants.h"
#include "Move.h"
#include "BitboardManipulation.h"
#include "GameState.h"


class Zobrist;

class Chess
{
public:
	Chess();
	~Chess();

	bool IsWhiteTurn() const;
	const GameState& GetGameState() const;

	//For Input/Output:
	PieceType GetPieceTypeAt(int square) const;
	PieceType GetPieceAt(int square, bool& isWhite) const;
	void PrintBoard(std::vector<Move>& moves) const;
	void PrintBoard() const;
	
	//Making Moves:
	void MakeMove(const Move move);
	void UndoMove();
	void CalculateLegalMoves(std::vector<Move>& moves);
	bool IsCheck() const;

	uint64_t GetPawnAttacks(bool isWhite, int square) const;
	uint64_t GetKnightAttacks(int square) const;
	uint64_t GetBishopAttacks(int square, uint64_t occupancy) const;
	uint64_t GetRookAttacks(int square, uint64_t occupancy) const;
	uint64_t GetKingAttacks(int square) const;

	void GenerateMagicNumbers();

private:

	
	void CalculateMoves(std::vector<Move>& moves) const;
	void SetUpBoard();
	
	inline int GetKingSquare(bool isWhite);

	void AddPotentialPromoteMove(std::vector<Move>& moves, Move move, int rank, int promotionRank) const;
	bool IsMoveCheck(int startSquare, int endSquare, PieceType pieceType, bool isWhite);
	bool IsMovePin(int startSquare, int endSquare, PieceType pieceType, bool isWhite);

	bool IsSquareAttacked(int square, bool isWhite) const;
	bool IsSquareAttackedBy(int square, bool isWhite, PieceType pieceType, uint64_t occupancy);

	void CalculatePawnMoves(uint64_t pawns, const uint64_t* occupancies, std::vector<Move>& moves, const bool isWhite) const;
	inline void BitBoardToMoves(const uint64_t bitboard, std::vector<Move>& moves, const PieceType pieceType, const uint32_t originSquare) const;

	//Attack patterns:
	static uint64_t CalculatePawnAttacks(bool isWhite, int square);
	static uint64_t CalculateKnightAttacks(int square);
	static uint64_t CalculateKingAttacks(int square);
	static uint64_t CalculateBishopAttacks(int square);
	static uint64_t CalculateRookAttacks(int square);
	static uint64_t CalculateBlockedBishopAttacks(int square, uint64_t occupancy);
	static uint64_t CalculateBlockedRookAttacks(int square, uint64_t occupancy);

	uint64_t CalculateMagicNumber(int square, int attackCount, bool isBishop);
	void InitSliderAttacks(bool isBishop);

private:

	static uint64_t pawnAttacks[2][64];
	static uint64_t knightAttacks[64];
	static uint64_t kingAttacks[64];

	static uint64_t bishopAttacks[64];
	static uint64_t rookAttacks[64];
	static uint64_t bishopBlockedAttacks[64][512];
	static uint64_t rookBlockedAttacks[64][4096];

	bool isWhiteTurn = true;
	std::stack<GameState> log;
	Zobrist* zobrist;
};
