#include "uci.h"

uci::uci() {
	cout << "id name Heureka Engine" << endl;
	cout << "id author SimonHetzer" << endl;
	cout << "id version 0.1" << endl;
	cout << "uciok" << endl;

	srand(time(NULL));

	mainLoop();
}

void uci::handleInputLoop() {
	while (true) {
		if (waitingForBoard) {
			output = "info depth 1\n";
			// Board has finished searching
			if (!board.processing) {
				output = "bestmove " + Move::toString(board.currentSearch.bestMove) + "\n";
				waitingForBoard = false;
				if (searchThread.joinable())
					searchThread.join();
			}
			// Protocol forces board to stop searching
			// Use best move you found till now
			if (input == "stop") {
				output = "bestmove " + Move::toString(board.currentSearch.bestMove) + "\n";
				waitingForBoard = false;
				board.stopDemanded = true;
				if (searchThread.joinable())
					searchThread.join();
				input.clear();
			}
		}

		if (!input.empty()) {
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
				output = "readyok\n";
			}
			else if (input == "quit") {
				searchThread.join();
				break;
			}
			input.clear();
		}
		this_thread::sleep_for(100ms);
	}
	running = false;
}

void uci::mainLoop() {
	running = true;

	// Create thread that continuesly reads input
	thread inputReader(&uci::inputLoop, this);

	// Create thread that continuesly parses input
	thread inputProcessor(&uci::handleInputLoop, this);

	while (running) {
		if (!output.empty()) {
			cout << output;
			output.clear();
		}
	}
	inputProcessor.join();
	inputReader.join();
}

void uci::inputLoop() {
	while (running) {
		if (input.empty()) {
			getline(cin, input);
		}
		// Wait for sub thread to process input
		this_thread::sleep_for(10ms);
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

// go wtime 300000 btime 300000 movestogo 40
void uci::parseGo(std::string input) {
	// Start search thread for ~4s
	searchThread = board.launchSearchThread(4000.0f);
	waitingForBoard = true;
}
