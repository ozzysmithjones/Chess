#pragma once
#include <stdint.h>
#include <random>


class Random
{

public:

	bool GetRandomBool();

	template<typename T>
	T GetRandomInt(const T min = std::numeric_limits<T>::min(), const T max = std::numeric_limits<T>::max())
	{
		std::uniform_int_distribution<T> dist(min, max);
		return dist(mersenneTwister);
	}

	template<typename T>
	T GetRandomReal(const T min = std::numeric_limits<T>::min(), const T max = std::numeric_limits<T>::max())
	{
		std::uniform_real_distribution<T> dist(min, max);
		return dist(mersenneTwister);
	}

	Random(uint64_t seed);
	Random();

private:

	std::mt19937_64 mersenneTwister;
};
