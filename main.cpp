#include "ChessGraphics.h"
#include "uci.h"

int main() {
	std::cout << "Welcome to Heureka Engine (Version 0.2), developed by Simon Hetzer.\n";
	std::cout << "Enter \"uci\" to start UCI communication (for debugging or Chess GUIs only).\n";
	std::cout << "Press any other key to launch integrated GUI.\n";

	string line;

	getline(cin, line);

	if (line == "uci") {
		// Creates UCI object that handles coming uci communication
		uci interface;
	}
	else {
		// Constructs the gui which enters the main loop
		ChessGraphics gui;
	}
	return 0;
}
