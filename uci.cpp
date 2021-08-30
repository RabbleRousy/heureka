#include "uci.h"

uci::uci() {
	cout << "id name myChessEngine" << endl;
	cout << "id author SimonHetzer" << endl;
	cout << "uciok" << endl;

	srand(time(NULL));

	loop();
}

void uci::loop() {

	string input;
	while (true) {
		input = "";
		getline(cin, input);
		if (input == "ucinewgame") {
			board.readPosFromFEN();
		}
		else if (input.substr(0, 8) == "position") {
			parsePosition(input);
		}
		else if (input.substr(0, 2) == "go") {
			parseGo(input);
		}
		else if (input == "isready") {
			// Check some things ...
			// ....
			cout << "readyok" << endl;
		}
		else if (input == "quit") {
			break;
		}
	}
}

void uci::parsePosition(std::string input) {
	if (input.substr(0, 17) == "position startpos") {
		// Load start position
		board.readPosFromFEN();
	}
	else {
		// Parse a FEN string
		std::string fen = input.substr(9, input.find("moves") - 9);
		board.readPosFromFEN(fen);
	}
	int i = input.find("moves");
	if (i == string::npos) {
		return;
	}
	// Skip "moves "
	i += 6;
	while ((i + 3) < input.size()) {
		int wordEnd = input.find(' ', i);
		if (wordEnd == -1) wordEnd = input.size();
		std::string move = input.substr(i, wordEnd - i);
		cout << "Trying to parse move: " << move << '\n';
		board.doMove(move);
		board.swapCurrentPlayer();
		i = wordEnd + 1;
	}
}

void uci::parseGo(std::string input) {
	board.generateMoves();
	cout << "bestmove " << Move::toString(board.possibleMoves[rand() % board.possibleMoves.size()]) << '\n';
}
