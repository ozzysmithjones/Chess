#pragma once
#include <stdint.h>
#include <limits>

inline bool GetBit(uint64_t bitboard, const uint32_t index)
{
	return bitboard & (1ull << index);
}

inline void SetBit(uint64_t& bitboard, const uint32_t index)
{
	bitboard = bitboard | (1ull << index);
}

inline void PopBit(uint64_t& bitboard, const uint32_t index)
{
	bitboard = bitboard & ~(1ull << index);
}

inline const int ToIndex(const int x, const int y)
{
	return (y << 3) | x;
}

inline void ToCoord(const int index, int& x, int& y)
{
	x = index & 7;
	y = index >> 3;
}

void PrintBitboard(uint64_t bitboard);
unsigned GetBitCount(uint64_t bitboard);
unsigned GetLeastIndex(uint64_t bitboard);

uint64_t CalculatePermutation(int permutationIndex, int maskBitCount, uint64_t mask);
