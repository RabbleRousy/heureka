#pragma once
#include <vector>
#include <mlpack/core.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
#include <ensmallen_bits/gradient_descent/gradient_descent.hpp>
#include "Board.h"

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

	void recalculateAccumulator(const std::vector<int> &activeFeatures, bool white);
	void updateAccumulator(const std::vector<int>& removedFeatures, const std::vector<int>& addedFeatures, bool white);

	template <int inputSize, int outputSize>
	void linear(const Linear<inputSize, outputSize>& layer, const float* input, float* output);
	void clippedReLu(int size, const float* input, float* output);

	std::string getHalfKPcoordinateList(unsigned long long row, Board* board);
	void getHalfKPvector(bool white, char* features, Board* board);

	void loadModel(std::string path);

public:
	NNUE();
	NNUE(std::string modelPath);
	float evaluate(bool whiteToMove);
	void train();
	void formatDataset(std::string path);
	void predictTest();
};

