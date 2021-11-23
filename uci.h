#pragma once

#include "Board.h"
#include "TranspositionTable.h"
#include <iostream>
#include <string>
#include <time.h>
#include <mutex>


using namespace std;

class uci
{
private:
	bool running;
	Board board;
	bool waitingForBoard;
	string input, output;
	mutex ioMutex;
	thread searchThread;
	future<Board::SearchResults> searchResults;

public:
	uci();
	void mainLoop();
	string getWordAfter(const string& s, const string& w);
	void handleInputLoop();
	void inputLoop();
	void parsePosition(string input);
	void parseGo(string input);
	void parseOption(string input);
};

