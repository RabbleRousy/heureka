#pragma once

#include "Board.h"
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
	std::future<Board::SearchResults> searchResults;

public:
	uci();
	void mainLoop();
	void handleInputLoop();
	void inputLoop();
	void parsePosition(std::string input);
	void parseGo(std::string input);
};

