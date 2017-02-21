#include <iostream>
#include <sstream>
#include <string>
#include "picosha2.h"
using namespace std;

string sha256(const string& src) {
	return picosha2::hash256_hex_string(src);
}

void feed_prng(const char* previous, int nonce) {
	stringstream ss;
	ss << previous << nonce;
	cout << "Hashing: " << ss.str() << endl;
	string hash = sha256(ss.str());
	cout << "Result: " << hash << endl;
}

extern "C" {
	int solve_sorted_list(const char* target_prefix,
			const char* previous_hash,
			int nb_elements) {
		int nonce = 0;
		feed_prng(previous_hash, nonce);
		return nonce;
	}
}
