#pragma once

#include "Board.h"
#include <iostream>
#include <string>
#include <time.h>

using namespace std;

class uci
{
private:
	Board board;

public:
	uci();
	void loop();
	void parsePosition(std::string input);
	void parseGo(std::string input);
};

