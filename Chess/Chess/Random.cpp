#include "Random.h"

bool Random::GetRandomBool()
{
	static std::uniform_int_distribution<int> dist(0, 1);
	return dist(mersenneTwister);
}

Random::Random(uint64_t seed)
{
	mersenneTwister.seed(seed);
}

Random::Random()
{
	std::uint_fast32_t seed_data[std::mt19937::state_size];
	std::random_device r;
	std::generate_n(seed_data, std::mt19937::state_size, std::ref(r));
	std::seed_seq q(std::begin(seed_data), std::end(seed_data));

	mersenneTwister.seed(q);
}
