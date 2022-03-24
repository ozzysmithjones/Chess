#include <iostream>
#include <bitset>
#include <nmmintrin.h>
#include "BitboardManipulation.h"

void PrintBitboard(uint64_t bitboard)
{
	std::wcout << L"\n";
	int index = 0;
	for (int y = 0; y < 8; y++)
	{
		std::wcout << (8 - y) << L": ";

		for (int x = 0; x < 8; x++, index++)
		{
			std::wcout << GetBit(bitboard, index) << L" ";
		}

		std::wcout << L"\n";
	}

	std::wcout << L"   A B C D E F G H\n";
	std::wcout << L"Decimal value = " << std::dec << bitboard << L"\n";
	std::wcout << L"Hex value = " << std::hex << bitboard << L"\n";
	std::wcout << L"NOT Hex value = " << std::hex << ~bitboard << L"\n\n" << std::dec;
}

constexpr uint64_t m1 = 0x5555555555555555; //binary: 0101...
constexpr uint64_t m2 = 0x3333333333333333; //binary: 00110011..
constexpr uint64_t m4 = 0x0f0f0f0f0f0f0f0f; //binary:  4 zeros,  4 ones ...
constexpr uint64_t h01 = 0x0101010101010101; //the sum of 256 to the power of 0,1,2,3...

#define USE_POPCNT

inline unsigned GetBitCount(uint64_t x)
{

#ifndef USE_POPCNT

	// Method 1 (simple)

	/*
	unsigned count = 0u;
	while (x)
	{
		++count;
		x &= x - 1;
	}

	return count;
	*/

	x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
	x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits 
	x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits 
	return (x * h01) >> 56;			//returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ... 

#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)

	return (int)_mm_popcnt_u32(x >> 32) + (int)_mm_popcnt_u32((int32_t)x);

#else

	return __builtin_popcountll(x);

#endif
}

unsigned GetLeastIndex(uint64_t bitboard)
{
	if (bitboard)
	{
		return GetBitCount((bitboard & -(int64_t)bitboard) - 1);
	}

	return -1;
}

uint64_t CalculatePermutation(int permutationIndex, int maskBitCount, uint64_t mask)
{
	uint64_t occupancy = 0ull;
	for (int i = 0; i < maskBitCount; i++)
	{
		unsigned square = GetLeastIndex(mask);
		PopBit(mask, square);

		if (permutationIndex & (1ull << i))
		{
			occupancy |= (1ull << square);
		}
	}

	return occupancy;
}