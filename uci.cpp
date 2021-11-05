#include "uci.h"

uci::uci() {
	cout << "id name Heureka Engine" << endl;
	cout << "id author SimonHetzer" << endl;
	cout << "id version 0.2" << endl;
	cout << "uciok" << endl;

	srand(time(NULL));

	mainLoop();
}

void uci::handleInputLoop() {
	unsigned int d = 0, p = 0;
	int e = 0;
	Move* m;

	while (true) {
		ioMutex.lock();
		if (waitingForBoard) {
			// Print update of new searched depth
			/*
			d = board.currentSearch.depth;
			p = board.currentSearch.positionsSearched;
			e = board.currentSearch.evaluation;
			m = &board.currentSearch.bestMove;

			output += "info depth " + std::to_string(d);
			output += " score cp " + std::to_string(e);
			output += " pv " + Move::toString(*m);
			output += " nodes " + std::to_string(p) + '\n';
			*/

			// Board has finished searching
			if (searchResults._Is_ready()) {
				Board::SearchResults results = searchResults.get();
				output += "bestmove " + Move::toString(results.bestMove) + "\n";
				waitingForBoard = false;
			}
			// Protocol forces board to stop searching
			// Use best move you found till now
			if (input == "stop") {
				board.stopDemanded = true;
				// Future will be ready immediately, because stop was demanded
				Board::SearchResults results = searchResults.get();
				output += "bestmove " + Move::toString(results.bestMove) + "\n";
				waitingForBoard = false;
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
				output += "readyok\n";
			}
			else if (input == "quit") {
				ioMutex.unlock();
				break;
			}
			input.clear();
		}
		ioMutex.unlock();
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
			ioMutex.lock();

			size_t lineEnd = output.find_first_of('\n');
			string line = output.substr(0, lineEnd + 1);
			cout << line << flush;
			output = output.substr(lineEnd + 1, output.size() - lineEnd - 1);
		
			ioMutex.unlock();
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
		board.doMove(move);
		board.swapCurrentPlayer();
		i = wordEnd + 1;
	}
}

// go wtime 300000 btime 300000 movestogo 40
void uci::parseGo(std::string input) {
	// Start search thread for ~4s
	searchResults = std::async(&Board::iterativeSearch, board, 4000.0f);
	waitingForBoard = true;
}
