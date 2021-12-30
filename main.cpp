#include "ChessGraphics.h"
#include "uci.h"
#include "Testing.h"
#include "NNUE.h"

using namespace std;

int main() {
	cout << "Welcome to Heureka Engine (Version 0.2.4), developed by Simon Hetzer.\n";
	cout << "Enter \"uci\" to start UCI communication (for debugging or Chess GUIs only).\n";
	cout << "Enter \"test\" to run the current test suite.\n";
	cout << "Enter \"train\" to start a training session of the NNUE.\n";
	cout << "Enter \"format\" to format the given dataset for later use in training.\n";
	cout << "Enter \"predict\" to predict a testdata set with the given NNUE.\n";
	cout << "Press any other key to launch integrated GUI.\n";

	string line;

	getline(cin, line);

	if (line == "uci" || line == "xboard") {
		// Creates UCI object that handles coming uci communication
		uci interface;
	}
	else if (line == "test") {
		// Run Testsuite
		Testing test;
	}
	else if (line == "format") {
		NNUE nnue;
		cout << "Start index: ";
		cin >> line;
		int from = stoi(line);
		cout << "End index: ";
		cin >> line;
		int to = stoi(line);
		cout << "Data path: ";
		cin >> line;
		nnue.formatDataset(line, from, to);
	}
	else if (line == "train") {
		NNUE nnue;

		cout << "Train a NEW network? (Y for yes) ";
		cin >> line;

		string modelPath, dataPath;
		cout << "Model path: ";
		cin >> modelPath;

		cout << "Data path: ";
		cin >> dataPath;

		string buf;
		double stepSize;
		cout << "Step size: ";
		cin >> buf;
		stepSize = stod(buf);

		int batchSize;
		cout << "Batch size: ";
		cin >> buf;
		batchSize = stoi(buf);

		double tolerance;
		cout << "Tolerance: ";
		cin >> buf;
		tolerance = stod(buf);

		int maxIterations;
		cout << "Max iterations: ";
		cin >> buf;
		maxIterations = stoi(buf);

		cout << "Starting training with parameters:\n"
			<< "Model: " << modelPath << '\n' << "Data: " << dataPath << '\n'
			<< "Step size: " << to_string(stepSize) << ", batch size: " << to_string(batchSize) << '\n'
			<< "tolerance: " << to_string(tolerance) << ", max iterations: " << to_string(maxIterations) << '\n';

		nnue.train(line == "Y", modelPath, dataPath, stepSize, batchSize, tolerance, maxIterations);
	}
	else if (line == "predict") {
		NNUE nnue;

		string modelPath, dataPath;
		cout << "Model path: ";
		cin >> modelPath;

		cout << "Test data path: ";
		cin >> dataPath;

		nnue.predictTest(modelPath, dataPath);
	}
	else {
		// Constructs the gui which enters the main loop
		ChessGraphics gui;
	}
	return 0;
}
