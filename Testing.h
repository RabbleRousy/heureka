#pragma once

#include "Board.h"
#include <iostream>
#include <fstream>

using namespace std;

class Testing {
private:
	struct TestCase {
		const string name;
		const string fen;
		const unsigned int depth;

		TestCase(string n, string f, unsigned int d)
		: name(n), fen(f), depth(d) { }
	};

	const string resultsPath = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TestResults\\";

	const string VERSION_NAME = "0_2_3 FUTILITY PRUNING";

	const TestCase testCases[3] = {
		TestCase("Seb Lauge Testposition",
		"r3k2r/p1ppqpb1/Bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPB1PPP/R3K2R b KQkq",
		5),
		TestCase("CCR 1 hour #1",
		"rn1qkb1r/pp2pppp/5n2/3p1b2/3P4/2N1P3/PP3PPP/R1BQKBNR w KQkq",
		6),
		TestCase("CCR 1 hour #7",
		"r4rk1/3nppbp/bq1p1np1/2pP4/8/2N2NPP/PP2PPB1/R1BQR1K1 b - -",
		5)
	};


public:
	Testing();
	void runTest();
};

