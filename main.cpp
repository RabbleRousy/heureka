#include "ChessGraphics.h"
#include "uci.h"

int main() {
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
