#include "NNUE.h"

void NNUE::recalculateAccumulator(const std::vector<int> &activeFeatures, bool white) {
	// Copy L0's bias
	for (int i = 0; i < M; i++) {
		accumulator[white][i] = L0.biases[i];
	}
	// Add the weights for active feature's column
	for (int a : activeFeatures) {
		for (int i = 0; i < M; i++) {
			accumulator[white][i] += L0.weights[a][i];
		}
	}
}

void NNUE::updateAccumulator(const std::vector<int>& removedFeatures, const std::vector<int>& addedFeatures, bool white) {
	// Subtract weights of removed Features
	for (int r : removedFeatures) {
		for (int i = 0; i < M; i++) {
			accumulator[white][i] -= L0.weights[r][i];
		}
	}
	// Add weights of added features
	for (int a : addedFeatures) {
		for (int i = 0; i < M; i++) {
			accumulator[white][i] += L0.weights[a][i];
		}
	}
}

void NNUE::clippedReLu(int size, const float* input, float* output) {
	for (int i = 0; i < size; i++) {
		// Clip value between 0 and 1
		output[i] = std::max(std::min(input[i], 1.0f), 0.0f);
	}
}

float NNUE::evaluate(bool whiteToMove) {
	float input[2 * M];
	// Use the accumulator to fill the input vector
	for (int i = 0; i < M; i++) {
		input[i] = accumulator[whiteToMove][i];
		input[i + M] = accumulator[!whiteToMove][i];
	}

	float output[2 * M];
	// Activation function for accumulator results
	clippedReLu(2 * M, input, output);

	// First hidden layer (write output into old input buffer)
	linear<M * 2, K>(L1, output, input);
	clippedReLu(K, input, output);

	// Second hidden layer
	linear<K, K>(L2, output, input);
	clippedReLu(K, input, output);

	// Output layer
	linear<K, 1>(L3, output, input);

	return input[0];
}

void NNUE::train() {
	arma::mat dataset;
	mlpack::data::Load("trainingData.csv", dataset, true);

	mlpack::ann::NNUEnet network;
}

void NNUE::formatDataset(std::string path) {
	std::string line;
	std::string fen;
	std::ifstream file(path);

	// Create a board to help with fen reading
	Board board;
	// First line is header
	std::getline(file, line);

	// Feature vector, N byte chararray
	char* features = new char[2*N];

	while (std::getline(file, line)) {
		// label and value are comma-separated
		fen = line.substr(0, line.find(','));
		board.readPosFromFEN(fen);
		//board.print();
		std::string e = line.substr(line.find(',')+1);
		int eval = std::stoi(e);
		
		constructHalfKPvector(features, &board);
	}
}

void NNUE::constructHalfKPvector(char* features, Board* board) {
	// Clear feature vector
	memset(features, '0', sizeof(features));
}

template<int inputSize, int outputSize>
inline void NNUE::linear(const Linear<inputSize, outputSize>& layer, const float* input, float* output) {
	// "Add" Biases
	for (int i = 0; i < outputSize; i++) {
		output[i] = layer.biases[i];
	}

	// Vector-matrix multiplication: input[1xA] * weights[AxB] = output[1xB]
	for (int i = 0; i < inputSize; i++) {
		for (int j = 0; j < outputSize; j++) {
			output[j] += input[i] * layer.weights[i][j];
		}
	}
}
