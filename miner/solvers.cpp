#include <iostream>
#include <sstream>
#include <string>
#include <climits>
#include <vector>
#include <algorithm>
#include <iterator>
#include <chrono>
#include <future>
#include <thread>
#include <atomic>

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

template <class TaskCreator>
int multithreaded_task(int start, int max_tries, TaskCreator task_creator) {
	const auto n_threads = max(thread::hardware_concurrency(), 1u);
	cout << "Launching " << n_threads << " threads." << endl;
	vector<future<int>> threads;
	threads.reserve(n_threads);
	const auto before = chrono::steady_clock::now();
	const int workload = max_tries / n_threads;
	atomic_bool done(false);

	for (int i = 0; i < n_threads; ++i) {
		const int my_start = start + workload * i;
		const int my_end = my_start + workload;
		threads.push_back(async(launch::async,
					[i, my_start, my_end, workload, &task_creator, &done] {
					auto task = task_creator();
					for (int nonce = my_start; nonce < my_end; ++nonce) {
						for (int j = 0; j < 5000; ++j, ++nonce) {
							if (task(nonce)) {
								done.store(true);
								cout << "Thread " << i << " found nonce " << nonce << endl;
								return nonce;
							}
						}
						if (done) {
							return -(nonce - my_start);
						}
					}
					return -workload;
					}));
	}
	vector<int> results;
	transform(begin(threads), end(threads), back_inserter(results), [] (auto &thread) { return thread.get(); });
	const auto found_nonce = *max_element(begin(results), end(results));

	const auto after = chrono::steady_clock::now();
	const auto diff = chrono::duration_cast<chrono::duration<float>>(after - before);
	int explored = 0;
	for (int i = 0; i < n_threads; ++i) explored += results[i] >= 0 ? results[i] - workload * i - start : -results[i];
	cout << "speed: " << static_cast<double>(explored) / diff.count() << "/s" << endl;

	return found_nonce;
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
		string target(target_prefix);
		struct sort_task {
			string target;
			const char* previous_hash;
			int nb_elements;
			bool asc;
			MT64 mt;

			sort_task(const string& target, const char* previous_hash, int nb_elements, bool asc)
				: target{target}, previous_hash{previous_hash}, nb_elements{nb_elements}, asc{asc}, mt{} {}

			bool operator()(int nonce) {
				return test_list_sort_nonce(mt, target, previous_hash, nb_elements, asc, nonce);
			}
		};

		const int step_size = 150000;
		for (int nonce = 0; nonce < 99999999; nonce += step_size) {
			int found_nonce = multithreaded_task(nonce, step_size,
					[&] { return sort_task{target, previous_hash, nb_elements, asc}; });
			if (found_nonce >= 0) return found_nonce;
		}
		return 0;
	}
}
