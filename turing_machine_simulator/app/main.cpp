#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "turing_machine.h"

void block() {
	std::cout << std::endl;
	std::cout << "Enter 0 to end to end.";
	int end = 0;
	std::cin >> end;
}

std::string getOutFileName(const std::string& inFileName, 
	const std::string& suffix) 
{
	std::string outFileName;
	
	// Remove file extension from infile
	outFileName += inFileName.substr(0, inFileName.find("."));
	outFileName += "_autotool";

	// Add suffix if provided
	if (suffix.length() != 0)
		outFileName += "_" + suffix;

	return outFileName + ".txt";
}

int main(int argc, char** argv) {
	using namespace turing_machine_sim::turing_machine;
	
	// Load tm
	std::string inFilePath;
	std::cout << "Enter turing machine file to load: ";
	std::cin >> inFilePath;
	auto tm = loadTuringMachine(inFilePath);

	// Convert or run
	char choice = '\0';
	std::cout << "Convert or run? [c/r]: ";
	std::cin >> choice;
	
	switch (choice) {
	// Convert
	case 'c':
		// File suffix
		std::string suffix;
		std::cout << "Enter filename (additional) suffix: ";
		std::cin >> suffix;
		std::ofstream outFile(getOutFileName(inFilePath, suffix),
			std::ios::out | std::ios::trunc);
		outFile << tm.getAutotoolDescription();
		return 0;
	}

	// Iterate
	constexpr std::size_t MAX_ITERATIONS = 25;
	const auto numberOfIterations = (argc == 2)
		? std::stoi(argv[1])
		: MAX_ITERATIONS;
	for (int i = 0; i < MAX_ITERATIONS && !tm.didHalt(); ++i) {
		std::cout << tm << std::endl;
		tm.iterate();
	}
	std::cout << tm << std::endl;

	//block();
	return 0;
}
