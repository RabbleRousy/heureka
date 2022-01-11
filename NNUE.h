#pragma once
#include <direct.h>
#include <vector>
#include <mlpack/core.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
#include <ensmallen_bits/gradient_descent/gradient_descent.hpp>
#include "LinearSplit.hpp"
#include "ClippedReLU.h"

// Forward declaration for circular dependencies
class Board;
class PrintValidationLoss;

// 2*FeatureSet[N]->M*2->K->K->1
// 2*HalfKP[40960]->256x2->32->32->1
const int N = 40960;
const int M = 256;
const int K = 32;

class NNUE {
private:

	// Linear network layers
	template <int inputSize, int outputSize>
	struct Linear {
		int in_size, out_size;
		float** weights;
		float* biases;

		Linear() {
			in_size = inputSize;
			out_size = outputSize;
			// Construct the input x output Weights matrix
			weights = new float* [inputSize];
			for (int i = 0; i < inputSize; i++) {
				weights[i] = new float[outputSize];
			}

			biases = new float[outputSize];
		}

		~Linear() {
			for (int i = 0; i < inputSize; i++) {
				delete[] weights[i];
			}
			delete[] weights;
			delete[] biases;
		}
	};

	struct Accumulator {
		// These will be the input vector of L1
		float* v[2];

		float* operator[](bool white) {
			return v[white];
		}

		Accumulator() {
			v[0] = new float[M];
			v[1] = new float[M];
		}

		~Accumulator() {
			delete[] v[0];
			delete[] v[1];
		}
	};

	Accumulator accumulator;

	Linear<N, M> L0;
	Linear<M * 2, K> L1;
	Linear<K, K> L2;
	Linear<K, 1> L3;

	template <int inputSize, int outputSize>
	void linear(const Linear<inputSize, outputSize>& layer, const float* input, float* output);
	void relu(int size, const float* input, float* output);

	std::string getHalfKPcoordinateList(unsigned long long row, Board* board);
	void getHalfKPvector(bool white, char* features, Board* board);

	void loadModel(std::string path);

public:
	using lossFunction = mlpack::ann::MeanSquaredError<>;

	NNUE();
	NNUE(std::string modelPath);
	float evaluate(bool whiteToMove);
	void train(bool newNet, std::string modelPath, std::string dataPath, std::string valPath, double stepSize, int batchSize, int maxIterations);
	void train(bool newNet, std::string modelPath, std::array<int,10> batchCounts, double stepSize, int batchSize, int maxIterations,
				std::string valPath = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\validation_rdm_upper500k.csv",
				std::string batchesPath = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\random_evals\\");
	void autoTrain();
	void formatDataset(std::string inPath, std::string outPath, int from, int to);
	void predictTest(std::string modelPath, std::string testdataPath = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\validation_rdm_upper500k.csv",
		std::string outName = "\\predictions.csv");
	unsigned int getHalfKPindex(short perspective, short pieceType, short pieceColor, short square, short kingSquare);
	void recalculateAccumulators(const Board* board);
	void recalculateAccumulator(const std::vector<int>& activeFeatures, bool white);
	void updateAccumulator(const std::vector<int>& removedFeatures, const std::vector<int>& addedFeatures, bool white);
};

