#pragma once
#include <direct.h>
#include <vector>
#include <mlpack/core.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>
#include <ensmallen_bits/gradient_descent/gradient_descent.hpp>
#include "LinearBitSplit.hpp"
#include "ClippedReLU.h"

// Forward declaration for circular dependencies
class Board;
class PrintValidationLoss;

// 2*FeatureSet[N]->M*2->K->K->1
// 2*HalfKP[40960]->256x2->32->32->1
const int N = 41600;
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

	

	Linear<N, M> L1;
	Linear<M * 2, K> L2;
	Linear<K, K> L3;
	Linear<K, 1> L4;

	template <int inputSize, int outputSize>
	void linear(const Linear<inputSize, outputSize>& layer, const float* input, float* output);
	void crelu(int size, const float* input, float* output);

	std::string getHalfKPcoordinateList(unsigned long long row);

	void loadModel(std::string path);

public:
	using lossFunction = mlpack::ann::MeanSquaredError<>;

	/// <summary>
	/// One class which actually holds 2 accumulators for each perspective and allows access to them.
	/// </summary>
	struct Accumulator {
		// These will be the input vector of L2
		float v[2][M];

		/// <summary>
		/// Access one half of the 2 accumulators by perspective.
		/// </summary>
		/// <param name="white">true if white's perspective's accumulator is wanted</param>
		/// <returns>that perspective's accumulator as a floatarray</returns>
		float* operator[](bool white) {
			return v[white];
		}
	};

	Accumulator accumulator;

	// Struct that holds all the parameters for a training session started with the overloaded train() function
	struct TrainSession {
		const std::string trainDataPath, valDataPath;
		// How many calls to the train() function with different data matrices
		const int trainings;
		// Where to start getting the trainData from the unformatted file
		const int offset;
		// How much data to use in one call of train() function
		const int dataPerTraining;
		// Of how many continuous chunks from the original data the formatted data should be made of
		const int dataChunks;
		// How many epochs to run at max per train() call
		const int epochsPerTraining;
		// How many datapoints to shift between trainings, if shift < (dataPerTraining/dataChunks) then new training will contain "known" data
		const int shift;
		// Amount of data in one of the <dataChunks> chunks
		const int chunkSize;

		TrainSession(int t, int o, int d, int c, int e, float s, std::string trainPath, std::string valPath)
			: trainings(t), offset(o), dataPerTraining(d), dataChunks(c), epochsPerTraining(e),
			shift(d / c * s), chunkSize(shift* (t - 1) + (d / c)),
			trainDataPath(trainPath), valDataPath(valPath) {
		}
	};
	/// <summary>
	/// Construct a new NNUE object (no internal model is loaded, only for training!)
	/// </summary>
	NNUE();
	/// <summary>
	/// Construct a new NNUE object and call loadModel().
	/// </summary>
	/// <param name="modelPath">path to a trained model to copy the weights and biases from.</param>
	NNUE(std::string modelPath);
	/// <summary>
	/// Evaluate the position stored in the current accumulators by performing the forward pass through the network.
	/// </summary>
	/// <param name="whiteToMove">sorts the accumulators in the right order (stm first)</param>
	/// <returns>value between 0.0 and 1.0, indicating wether stm is losing or winning</returns>
	float evaluate(bool whiteToMove);
	/// <summary>
	/// Trains a NNUE with the given parameters on a specific formatted dataset.
	/// </summary>
	/// <param name="newNet">wether to create a new or load an existing net</param>
	/// <param name="modelPath">directory where the network is loaded from or where it will be created</param>
	/// <param name="dataPath">path to a .csv file containing a training data matrix formatted by formatDataset()</param>
	/// <param name="valPath">path to a .csv file containing a validation data matrix formatted by formatDataset()</param>
	/// <param name="stepSize">for the gradient descent step</param>
	/// <param name="batchSize">size of each mini-batch which make up one epoch</param>
	/// <param name="maxIterations">after which the training is stopped</param>
	/// <param name="predictPath">optional path to a folder to save the predictions created by cref="ValidationLoss" callback to</param>
	void train(bool newNet, std::string modelPath, std::string dataPath, std::string valPath, double stepSize, int batchSize, int maxIterations, std::string predictPath = "");
	/// <summary>
	/// Trains a NNUE with the given parameters on a specific unformatted dataset for a session of multiple train()-calls.
	/// </summary>
	/// <param name="newNet">wether to create a new or load an existing net</param>
	/// <param name="modelPath">directory where the network is loaded from or where it will be created</param>
	/// <param name="stepSize">for the gradient descent step</param>
	/// <param name="batchSize">size of each mini-batch which make up one epoch</param>
	/// <param name="sessionInfo">struct containing all the session parameters</param>
	void train(bool newNet, std::string modelPath, double stepSize, int batchSize, const TrainSession& sessionInfo);
	/// <summary>
	/// Formats a (fen,centipawn) dataset to a (halfKP,stmEval) sparse matrix to be used in training.
	/// </summary>
	/// <param name="inPath">path to a .csv file containing the (fen,centipawn) evaluations, line by line</param>
	/// <param name="outPath">path and name of the .csv file that will be created. Matrix is saved in arma::coord_ascii format</param>
	/// <param name="from">which line to start formatting</param>
	/// <param name="to">which line to format (exclusive)</param>
	void formatDataset(std::string inPath, std::string outPath, int from, int to);
	/// <summary>
	/// Makes the network predict on a given dataset, compare predictions and labels, and save the results.
	/// </summary>
	/// <param name="modelPath">directory where the network is loaded from</param>
	/// <param name="testdataPath">path to the formatted data to feed through the network</param>
	/// <param name="outName">optional name of the output file</param>
	void predictTest(std::string modelPath, std::string testdataPath = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\validation_rdm_upper500k.csv",
		std::string outName = "\\predictions.csv");
	/// <param name="square">the piece is on</param>
	/// <param name="pieceType">from Pawn (2) to Queen (6)</param>
	/// <param name="our"> piece (1) or theirs (0)</param>
	/// <returns>the HalfPiece Index between 0 and 41.535, calculated by: 65 * (10 * square + 5 * our + pieceType - 2)</returns>
	unsigned int getHalfPieceIndex(short perspective, short square, short pieceType, short our);
	/// <param name="perspective">of the position, black's perspective get's flipped</param>
	/// <param name="pieceType">from Pawn (2) to Queen (6)</param>
	/// <param name="pieceColor">to check if it's "our" piece</param>
	/// <param name="square">of the piece (0...63)</param>
	/// <param name="kingSquare">to construct the index relative to the kingPos</param>
	/// <returns>the HalfKP Index between 0 and 41.599, calculated by halfPieceIndex + <paramref name="kingSquare"/> + 1</returns>
	unsigned int getHalfKPindex(short perspective, short pieceType, short pieceColor, short square, short kingSquare);
	/// <summary>
	/// Recalculates a single accumulator from scratch for the given side.
	/// </summary>
	/// <param name="activeFeatures">list of the indeces of all active (1) HalfKP features</param>
	/// <param name="white">true if it's whites perspective/accumulator</param>
	void recalculateAccumulator(const std::vector<int>& activeFeatures, bool white);
	/// <summary>
	/// Updates one of the accumulators incrementally.
	/// </summary>
	/// <param name="removedFeatures">list of the indeces of all HalfKP features to be removed</param>
	/// <param name="addedFeatures">list of the indeces of all HalfKP features to be added</param>
	/// <param name="white">true if it's whites perspective/accumulator</param>
	void updateAccumulator(const std::vector<int>& removedFeatures, const std::vector<int>& addedFeatures, bool white);
	/// <summary>
	/// Prints a list of all halfPiece and halfKP combinations and their resulting indeces.
	/// </summary>
	void printHalfKPindeces();
};