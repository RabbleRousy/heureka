#pragma once
#include <vector>

// FeatureSet[N]->M*2->K->1
// HalfKP[40960]->256x2->32->32->1

class NNUE {
private:
	static const int N = 40960;
	static const int M = 256;
	static const int K = 32;


	// Linear network layers
	template <int inputSize, int outputSize>
	struct Linear {
		float weights[inputSize][outputSize];
		float biases[outputSize];
	};

	Linear<N, M> L0;
	Linear<M*2, K> L1;
	Linear<K, K> L2;
	Linear<K, 1> L3;

	struct Accumulator {
		// These will be the input vector of L1
		float v[2][M];

		float* operator[](bool white) {
			return v[white];
		}
	};

	Accumulator accumulator;

	void recalculateAccumulator(const std::vector<int> &activeFeatures, bool white);
	void updateAccumulator(const std::vector<int>& removedFeatures, const std::vector<int>& addedFeatures, bool white);

	template <int inputSize, int outputSize>
	void linear(const Linear<inputSize, outputSize>& layer, const float* input, float* output);
	void clippedReLu(int size, const float* input, float* output);

public:
	float evaluate(bool whiteToMove);
};

