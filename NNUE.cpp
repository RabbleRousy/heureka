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
	arma::sp_mat sparseMatrix;
	sparseMatrix.load("C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\chessDataFormatted.csv", arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();
	arma::mat data = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);

	// Cut the labels from the last row of the trainingData
	arma::mat labels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);

	mlpack::ann::FFN<mlpack::ann::MeanSquaredError<>> network;

	// L0
	network.Add<mlpack::ann::Linear<>>(2 * N, 2 * M);
	network.Add<mlpack::ann::ReLULayer<> >();
	// L1
	network.Add<mlpack::ann::Linear<> >(2 * M, K);
	network.Add<mlpack::ann::ReLULayer<> >();
	// L2
	network.Add<mlpack::ann::Linear<> >(K, K);
	network.Add<mlpack::ann::ReLULayer<> >();
	// L3
	network.Add<mlpack::ann::Linear<> >(K, 1);

	network.Train(data, labels);
}

void NNUE::formatDataset(std::string path) {
	std::string line;
	std::string fen;
	std::ifstream input(path);
	std::ofstream output(path.substr(0, path.find('.')) + "Formatted1k.csv");

	// Create a board to help with fen reading
	Board board;
	// First line is header
	std::getline(input, line);

	unsigned long long row = 0;

	while (std::getline(input, line) && row < 1000) {
		// label and value are comma-separated
		fen = line.substr(0, line.find(','));
		board.readPosFromFEN(fen);
		//board.print();
		std::string e = line.substr(line.find(',')+1);
		
		
		// Coordinate list format for sparse matrices:
		// <row> <column> <nonzero-value>
		std::string coordinateList = getHalfKPcoordinateList(row, &board);
		// Append value
		coordinateList += std::to_string(row) + " 81920 " + e + '\n';

		output << coordinateList;


		row++;
	}

	input.close();
	output.close();
}

std::string NNUE::getHalfKPcoordinateList(unsigned long long row, Board* board) {
	std::string cl;
	bool whiteToMove = board->currentPlayer == Piece::WHITE;

	// Perspective of side to move (first HalfKP)
	unsigned short kingSquare = whiteToMove ? board->whiteKingPos : board->blackKingPos;
	if (!whiteToMove) {
		// Flip position for black
		kingSquare ^= 56;
	}
	unsigned short pieceSquare;

	for (short piece = Piece::PAWN; piece <= Piece::QUEEN; piece++) {
		for (short ourColor = 0; ourColor <= 1; ourColor++) {
			short pieceColor = ourColor ? board->currentPlayer : Piece::getOppositeColor(board->currentPlayer);

			bitboard pieces = board->bb.getBitboard(piece | pieceColor);
			Bitloop(pieces) {
				pieceSquare = getSquare(pieces);
				if (!whiteToMove) {
					// Calculating black's perspective, flip
					pieceSquare ^= 56;
				}
				unsigned int halfKPindex = 640 * kingSquare + 10 * pieceSquare + 5 * ourColor + (piece - 2);
				cl += std::to_string(row) + ' ' + std::to_string(halfKPindex) + " 1.0\n";
			}
		}
	}

	// Perspective of the other side (second HalfKP)
	kingSquare = whiteToMove ?  board->blackKingPos : board->whiteKingPos;
	if (whiteToMove) {
		// Flip position for black
		kingSquare ^= 56;
	}

	for (short piece = Piece::PAWN; piece <= Piece::QUEEN; piece++) {
		for (short ourColor = 0; ourColor <= 1; ourColor++) {
			short color = ourColor ? board->currentPlayer : Piece::getOppositeColor(board->currentPlayer);
			bitboard pieces = board->bb.getBitboard(piece | color);
			Bitloop(pieces) {
				pieceSquare = getSquare(pieces);
				if (whiteToMove) {
					// Calculating black's perspective, flip
					pieceSquare ^= 56;
				}
				unsigned int halfKPindex = 40960 + 640 * kingSquare + 10 * pieceSquare + 5 * ourColor + (piece - 2);
				cl += std::to_string(row) + ' ' + std::to_string(halfKPindex) + " 1.0\n";
			}
		}
	}

	return cl;
}

void NNUE::getHalfKPvector(bool white, char* features, Board* board) {
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
