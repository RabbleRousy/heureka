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
		/*
		* FORMAT DATA BATCHES
		* 
		string path = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\random_evals.csv";

		for (int fifties = 0; fifties < 10; fifties++) {
			for (int k = 0; k < 50; k++) {
				string outputPath = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\random_evals\\" + to_string(fifties + 1) + '_' + to_string(k + 1) + ".csv";
				nnue.formatDataset(path, outputPath, 50000 * fifties + 1000 * k, 50000 * fifties + 1000 * (k + 1));
			}
		}*/
		
		cout << "Start index: ";
		cin >> line;
		int from = stoi(line);
		cout << "End index: ";
		cin >> line;
		int to = stoi(line);
		cout << "Data path: ";
		cin >> line;
		nnue.formatDataset(line, line.substr(0, line.find_last_of('.'))
			+ "Formatted" + to_string(from) + '_' + to_string(to) + ".csv", from, to);
	}
	else if (line == "train") {
		NNUE nnue;

		cout << "Train a NEW network? (Y for yes) ";
		cin >> line;
		bool newNet = line == 'Y';

		string modelPath, dataPath, validationPath;
		cout << "Model path: ";
		cin >> modelPath;

		array<int, 10> batchCounts;
		batchCounts.fill(0);

		for (int i = 0; i < 10; i++) {
			cout << "How many k of the " << (i + 1) << ". 50k do you want to use? ";
			cin >> line;
			try {
				batchCounts[i] = stoi(line);
			}
			catch (...) {
				break;
			}
		}

		cout << "Validation Data path: ";
		cin >> validationPath;

		string buf;
		double stepSize;
		cout << "Step size: ";
		cin >> buf;
		stepSize = stod(buf);

		int batchSize;
		cout << "Batch size: ";
		cin >> buf;
		batchSize = stoi(buf);

		int maxIterations;
		cout << "Max iterations: ";
		cin >> buf;
		maxIterations = stoi(buf);

		cout << "Starting training with parameters:\n"
			<< "Model: " << modelPath << '\n' << "Data: " << dataPath << '\n'
			<< "Step size: " << to_string(stepSize) << ", batch size: " << to_string(batchSize) << '\n'
			<< "Max iterations: " << to_string(maxIterations) << '\n';

		if (validationPath != "n")
			nnue.train(newNet, modelPath, batchCounts, stepSize, batchSize, maxIterations, validationPath);
		else
			nnue.train(newNet, modelPath, batchCounts, stepSize, batchSize, maxIterations);
	}
	else if (line == "predict") {
		NNUE nnue;

		string modelPath, dataPath;
		cout << "Model path: ";
		cin >> modelPath;

		cout << "Test data path: ";
		cin >> dataPath;

		if (dataPath != "n")
			nnue.predictTest(modelPath, dataPath);
		else
			nnue.predictTest(modelPath);
	}
	else {
		// Constructs the gui which enters the main loop
		ChessGraphics gui;
	}
	return 0;
}
