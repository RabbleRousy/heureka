#include "ChessGraphics.h"
#include "uci.h"
#include "Testing.h"

int main() {
	std::cout << "Welcome to Heureka Engine (Version 0.2.4), developed by Simon Hetzer.\n";
	std::cout << "Enter \"uci\" to start UCI communication (for debugging or Chess GUIs only).\n";
	std::cout << "Enter \"test\" to run the current test suite.\n";
	std::cout << "Press any other key to launch integrated GUI.\n";

	string line;

	getline(cin, line);

	if (line == "uci" || line == "xboard") {
		// Creates UCI object that handles coming uci communication
		uci interface;
	}
	else if (line == "test") {
		// Run Testsuite
		Testing test;
	}
	else {
		// Constructs the gui which enters the main loop
		ChessGraphics gui;
	}
	return 0;
}
