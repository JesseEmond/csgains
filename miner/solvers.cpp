#include <iostream>
#include <sstream>
#include <string>
#include <climits>
#include "picosha2.h"
#include "mt.h"
using namespace std;

template <typename T>
T swap_endian(T u)
{
	static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}

string sha256(const string& src) {
	return picosha2::hash256_hex_string(src);
}

MT64::seed_t seed_from_hash(const string& hash) {
	string prefix(begin(hash), next(begin(hash), 16)); // 8 bytes
	stringstream converter(prefix);
	MT64::seed_t seed;
	converter >> std::hex >> seed;
	cout << converter.good() << endl;
	return swap_endian(seed);
}

MT64 feed_prng(const char* previous, int nonce) {
	stringstream ss;
	ss << previous << nonce;
	string hash = sha256(ss.str());
	auto seed = seed_from_hash(hash);
	return MT64(seed);
}


extern "C" {
	int solve_sorted_list(const char* target_prefix,
			const char* previous_hash,
			int nb_elements) {
		int nonce = 0;
		auto mt = feed_prng(previous_hash, nonce);
		return nonce;
	}
}
