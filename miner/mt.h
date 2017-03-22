#ifndef MT_H
#define MT_H

#include <array>
#include <iostream>

class MT64 {
public:
	using seed_t = unsigned long long int;
	using value_t = seed_t;

private:
	enum {
		W = 64,
		N = 312,
		F = 6364136223846793005ULL,
		M = 156,
		R = 31,
		A = 0xB5026F5AA96619E9ULL,
		U = 29,
		D = 0x5555555555555555ULL,
		S = 17,
		B = 0x71D67FFFEDA60000ULL,
		T = 37,
		C = 0xFFF7EEE000000000ULL,
		L = 43,
		LOWER_MASK = 0x7fffffff,
		UPPER_MASK = 0xffffffff80000000ULL
	};
	using state_value_t = value_t;
	using state_t = std::array<state_value_t, N>;

	state_t state;
	int index;

	void twist() {
		for (unsigned int i = 0; i < N; ++i) {
			auto x = (state[i] & UPPER_MASK) +
				 (state[(i+1) % N] & LOWER_MASK);
			auto xA = x >> 1;

			if (x % 2 != 0)
				xA ^= A;

			state[i] = state[(i + M) % N] ^ xA;
		}
		index = 0;
	}

public:
	void seed(const seed_t &seed) {
		index = N;
		state[0] = seed;
		for (unsigned int i = 1; i < N; ++i) {
			state[i] = F * (state[i-1] ^ (state[i-1] >> (W-2))) + i;
		}
	}

	state_value_t next() {
		if (index == N) twist();

		auto y = state[index];
		y ^= ((y >> U) & D);
		y ^= ((y << S) & B);
		y ^= ((y << T) & C);
		y ^= (y >> L);

		++index;
		return y;
	}
};

#endif
