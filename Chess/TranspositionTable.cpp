#include "TranspositionTable.h"


TranspositionTable::TranspositionTable()
{
	Clear();
}


void TranspositionTable::Clear()
{
	for (int32_t i = 0; i < numElements; ++i)
	{
		Element& element = elements[i];

		element.zobristKey = 0;
		element.depth = 0;
		element.scoreType = ScoreType::Exact;
		element.score = 0;
		element.move = 0;
	}
}


void TranspositionTable::Set(uint64_t zobristKey, int32_t depth, int32_t score, ScoreType scoreType, Move bestMove)
{
	if (score == 0)
		return;

	Element& element = elements[(uint32_t)zobristKey & (numElements - 1)];

	element.zobristKey = zobristKey;
	element.depth = depth;
	element.scoreType = scoreType;
	element.score = score;
	element.move = bestMove;
}

std::optional<int32_t> TranspositionTable::Get(uint64_t zobristKey, int32_t depth, int32_t alpha, int32_t beta, Move& move)
{
	Element& element = elements[(uint32_t)zobristKey & (numElements - 1)];
	move = 0;

	if (element.zobristKey == zobristKey)
	{
		move = element.move;

		if (element.depth >= depth)
		{
			switch (element.scoreType)
			{
			case ScoreType::Exact:
				return element.score;
			case ScoreType::Alpha:
				return element.score <= alpha ? std::optional<int32_t>(alpha) : std::nullopt;
			case ScoreType::Beta:
				return element.score >= beta ? std::optional<int32_t>(beta) : std::nullopt;
			}
		}
	}

	return std::nullopt;
}