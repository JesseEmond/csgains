#include <iostream>
#include <sstream>
#include <string>
#include <climits>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>

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
	int solve_list_sort(const char* target_prefix,
			const char* previous_hash,
			int nb_elements,
			bool asc) {
		int attempts = 0;
		auto last = chrono::steady_clock::now();

		int nonce = 0;
		string target(target_prefix);
		for (int nonce = 0; nonce < 9999999; ++nonce) {
			auto mt = feed_prng(previous_hash, nonce);
			vector<MT64::value_t> values(nb_elements, 0);
			generate(begin(values), end(values),
					[&] { return mt.next(); });
			if (asc) sort(begin(values), end(values));
			else sort(rbegin(values), rend(values));

			stringstream ss;
			copy(begin(values), end(values), ostream_iterator<MT64::value_t>(ss));
			auto hash = sha256(ss.str());
			if (hash.compare(0, target.size(), target) == 0) {
				cout << "YES!!!" << endl;
				return nonce;
			}

			++attempts;
			if (attempts % 10 == 0 && chrono::steady_clock::now() - last > 1s) {
				cout << "speed: " << attempts << "/s" << endl;
				attempts = 0;
				last = chrono::steady_clock::now();
			}
		}
		return nonce;
	}
}
