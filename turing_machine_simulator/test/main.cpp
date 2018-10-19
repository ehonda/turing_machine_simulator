#include <iostream>

#include "gtest/gtest.h"

void block() {
	std::cout << "Press Enter to end" << std::endl;
	std::cin.get();
}

int main(int argc, char **argv) {
	std::cout << "Running tests" << std::endl;

	testing::InitGoogleTest(&argc, argv);
	int testResults = RUN_ALL_TESTS();

	block();
	return testResults;
}
