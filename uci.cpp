#include "uci.h"

UCI::UCI() {
	cout << "id name Heureka Engine" << endl;
	cout << "id author SimonHetzer" << endl;
	cout << "id version 0.2.4" << endl;
	cout << "option name Hash type spin default 128 min 16 max " << TranspositionTable::maxMB << endl;
	cout << "uciok" << endl;

	srand(time(NULL));

	mainLoop();
}

void UCI::handleInputLoop() {
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
			else if (input.substr(0, 9) == "setoption") {
				parseOption(input);
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

void UCI::mainLoop() {
	running = true;

	// Create thread that continuesly reads input
	thread inputReader(&UCI::inputLoop, this);

	// Create thread that continuesly parses input
	thread inputProcessor(&UCI::handleInputLoop, this);

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

void UCI::inputLoop() {
	while (running) {
		if (input.empty()) {
			getline(cin, input);
		}
		// Wait for sub thread to process input
		this_thread::sleep_for(10ms);
	}
}

void UCI::parsePosition(std::string input) {
	if (input.substr(0, 17) == "position startpos") {
		// Load start position
		board.readPosFromFEN();
	}
	else {
		// Parse a FEN string
		string fen = input.substr(input.find("fen") + 4, input.length());
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
		i = wordEnd + 1;
	}
}

string UCI::getWordAfter(const string& sentence, const string& word) {
	size_t startIndex = sentence.find(word);
	if (startIndex == string::npos)
		return "";
	size_t wordLength = word.length() + 1;
	size_t searchStart = startIndex + wordLength;
	size_t searchEnd = sentence.find(' ', searchStart);
	return sentence.substr(searchStart, searchEnd - searchStart);
}

// go wtime 300000 btime 300000 movestogo 40
void UCI::parseGo(string input) {
	float movetime = 5000.0f;
	float wtime = 5000.0f;
	float btime = 5000.0f;
	unsigned int movestogo = 1;

	string word = getWordAfter(input, "movetime");
	if (!word.empty()) {
		movetime = stof(word);
		goto search;
	}

	word = getWordAfter(input, "wtime");
	if (!word.empty()) {
		wtime = stof(word);
	}

	word = getWordAfter(input, "btime");
	if (!word.empty()) {
		btime = stof(word);
	}

	word = getWordAfter(input, "movestogo");
	if (!word.empty()) {
		movestogo = stoi(word);
	}

	// Calculate time for search w.r.t. collected parameters
	movetime = Board::gameState.whiteToMove() ? wtime : btime;
	movetime /= movestogo;

	search:
	// Margin time
	if (movetime > 550.0f) movetime -= 500.0f;

	// Start search thread for ~4s
	searchResults = async(&Board::iterativeSearch, board, movetime);
	waitingForBoard = true;
}

void UCI::parseOption(std::string input) {
	string optionType = getWordAfter(input, "name");
	DEBUG_COUT("Option Name: " + optionType + '\n');

	if (optionType == "Hash") {
		string value = getWordAfter(input, "value");
		unsigned int size;
		try {
			size = stoi(value);
		}
		catch (exception e) {
			return;
		}
		TranspositionTable::setSize(size);
		output += "info transposition table size " + to_string(size) + " mb.\n";
	}
}
