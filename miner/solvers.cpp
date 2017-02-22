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

MT64 feed_prng(MT64& mt, const char* previous, int nonce) {
	stringstream ss;
	ss << previous << nonce;
	string hash = sha256(ss.str());
	auto seed = seed_from_hash(hash);
	mt.seed(seed);
}
bool test_list_sort_nonce(MT64& mt, const string& target, const char* previous_hash, int nb_elements, bool asc, int nonce) {
	feed_prng(mt, previous_hash, nonce);
	vector<MT64::value_t> values(nb_elements, 0);
	generate(begin(values), end(values), [&] { return mt.next(); });

	if (asc) sort(begin(values), end(values));
	else sort(begin(values), end(values), std::greater<MT64::value_t>());

	stringstream ss;
	copy(begin(values), end(values), ostream_iterator<MT64::value_t>(ss));
	auto hash = sha256(ss.str());
	return hash.compare(0, target.size(), target) == 0;
}


extern "C" {
	void unit_test() {
		MT64 mt;
		if (!test_list_sort_nonce(mt, "433e", "9cc5a925757e626b1febbdf62c1643d5bab6473c0a960ad823ab742e18560977", 100, false, 15236)) {
			class unittest_failed_list_sort{};
			throw unittest_failed_list_sort{};
		}

		cout << "Unit tests passed." << endl;
	}
	int solve_list_sort(const char* target_prefix,
			const char* previous_hash,
			int nb_elements,
			bool asc) {
		int attempts = 0;
		auto last = chrono::steady_clock::now();

		int nonce = 0;
		MT64 mt;
		string target(target_prefix);
		for (int nonce = 0; nonce < 9999999; ++nonce) {
			if (test_list_sort_nonce(mt, target, previous_hash, nb_elements, asc, nonce)) {
				cout << "Found matching nonce: " << nonce << endl;
				return nonce;
			}

			++attempts;
			if (attempts % 10 == 0 && chrono::steady_clock::now() - last > 5s) {
				cout << "speed: " << attempts/5.0 << "/s" << endl;
				attempts = 0;
				last = chrono::steady_clock::now();
			}
		}
		return nonce;
	}
}
