#include <iostream>
#include <sstream>
using namespace std;

void feed_prng(const char* previous, int nonce) {
	stringstream ss;
	ss << previous << nonce;
	cout << "Hashing: " << ss.str() << endl;
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
