#include <iostream>

void foo() {
	std::cout << "Hello World!" << std::endl;
}

extern "C" {
	void solve_sorted_list() { foo(); }
}
