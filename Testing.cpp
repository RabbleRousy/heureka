#include "Testing.h"

Testing::Testing() {
	runTest();
}

void Testing::runTest() {
	cout << "Running Test Suite for Version \"" << VERSION_NAME << "\"...\n";
	Board board;

	ofstream file;
	file.open(resultsPath + VERSION_NAME + ".csv");

	cout << "Opened file at " << resultsPath << VERSION_NAME << ".csv\n";

	file << ";;;" << VERSION_NAME << ";\n";
	file << "Testposition Name;FEN;Tiefe;\n";


	std::chrono::time_point<std::chrono::steady_clock> start, end;
	std::chrono::duration<float> duration;

	for (int i = 0; i < 3; i++) {
		const TestCase* testCase = testCases + i;

		cout << "Running Testcase \"" << testCase->name << "...\n";

		if (!board.readPosFromFEN(testCase->fen))
			continue;

		file << testCase->name << ';' << testCase->fen << ';' << testCase->depth << '\n';
		file << ";;;;\n"; // maybe unnecesary

		start = std::chrono::high_resolution_clock::now();
		Board::SearchResults searchResults = board.searchBestMove(testCase->depth);
		end = std::chrono::high_resolution_clock::now();
		duration = end - start;

		file << ";Ergebnisse;Best Move;" << Move::toString(searchResults.bestMove) << ";\n";
		file << ";;Evaluation (CP);" << searchResults.evaluation << ";\n";
		file << ";;Positionen;" << searchResults.positionsSearched << ";\n";
		string dur = to_string(duration.count() * 1000.0f);
		dur.replace(dur.find('.'), 1, ",");
		file << ";;Zeit in ms;" << dur << ";\n";

		file << ";;;;\n"; // maybe unnecesary
		file << ";;;;\n";
		file << ";;;;\n";

		if (i < 2) board.reset();
	}

	file.close();  
	cout << "All tests finished.";
}
