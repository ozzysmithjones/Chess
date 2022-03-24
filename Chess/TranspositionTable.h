#pragma once
#include <optional>
#include <stdint.h>
#include "Chess/Move.h"


enum class ScoreType
{
	Exact,
	Alpha, // at most
	Beta, // at least (beta cut off)
};


class TranspositionTable
{
	struct Element
	{
		uint64_t zobristKey;
		int32_t depth;
		ScoreType scoreType;
		int32_t score;
		Move move;
	};

public:

	TranspositionTable();

	void Clear();
	void Set(uint64_t zobristKey, int32_t depth, int32_t score, ScoreType scoreType, Move bestMove = 0);
	std::optional<int32_t> Get(uint64_t zobristKey, int32_t depth, int32_t alpha, int32_t beta, Move& move);

private:

	static const std::size_t numElements = 0x400000;
	Element elements[numElements];
};

