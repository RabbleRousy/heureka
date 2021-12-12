#include "NNUE.h"

NNUE::NNUE()  {}

NNUE::NNUE(std::string modelPath) {
	loadModel(modelPath);
}

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
	sparseMatrix.load("C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\tactic_evalsFormatted50k.csv", arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();
	arma::mat data = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);

	// Cut the labels from the last row of the trainingData
	arma::mat labels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);

	
	mlpack::ann::FFN<mlpack::ann::MeanSquaredError<>> network;
	/*
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

	*/
	
	mlpack::data::Load("C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainedNets\\FC1\\FCv2.bin", "network", network);

	network.Train(data, labels, ens::GradientDescent::GradientDescent(), ens::PrintLoss(), ens::ProgressBar());

	mlpack::data::Save("C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainedNets\\FC1\\FCv3.bin", "added_tactic50k", network, false);
}

void NNUE::formatDataset(std::string path) {
	std::string line;
	std::string fen;
	std::ifstream input(path);
	std::ofstream output(path.substr(0, path.find('.')) + "Formatted50k.csv");

	// Create a board to help with fen reading
	Board board;
	// First line is header
	std::getline(input, line);

	unsigned long long row = 0;

	while (std::getline(input, line) && row < 50000) {
		// label and value are comma-separated
		fen = line.substr(0, line.find_first_of(','));
		board.readPosFromFEN(fen);
		//board.print();
		std::string e = line.substr(line.find_first_of(',')+1);
		size_t i = e.find(',');
		if (i != std::string::npos) {
			e = e.substr(0, i);
		}
		float eval;

		if (e.find('#') != std::string::npos) {
			// Forced mate
			if (e.find('-') != std::string::npos) {
				// Black wins
				eval = (board.currentPlayer == Piece::BLACK) ? 1.0f : 0.0f;
			}
			else {
				// White wins
				eval = (board.currentPlayer == Piece::WHITE) ? 1.0f : 0.0f;
			}
		}
		else {
			eval = std::stof(e);
			if (board.currentPlayer == Piece::BLACK) {
				eval = -eval;
			}
			// Transform evaluation from centipawns to win/draw/loss (0/0.5/1.0)
			eval = utils::math::sigmoid(eval, 0, 1.0f / 410.0f);
		}
		
		
		// Coordinate list format for sparse matrices:
		// <row> <column> <nonzero-value>
		std::string coordinateList = getHalfKPcoordinateList(row, &board);
		// Append value
		coordinateList += std::to_string(row) + " 81920 " + std::to_string(eval) + '\n';

		output << coordinateList;


		row++;
	}

	input.close();
	output.close();
}

void NNUE::predictTest() {
	arma::sp_mat sparseMatrix;
	sparseMatrix.load("C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\chessDataFormatted1k.csv", arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();

	arma::mat data = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);

	arma::mat prediction;
	mlpack::ann::FFN<> network;
	mlpack::data::Load("C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainedNets\\FC1\\FCv3.bin", "FullyConnected_v3", network);


	for (int i = 0; i < data.n_cols; i++) {
		auto column = data.col(i);
		network.Predict(column, prediction);
		std::cout << "Prediction for #" << i << " : ";
		prediction.print();
		std::cout << '\n';
	}
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

void NNUE::loadModel(std::string path) {
	mlpack::ann::FFN<> model;
	mlpack::data::Load(path, "model", model);

	auto layer = model.Model()[0];
	
	// L0
	arma::mat parameters;
	boost::apply_visitor(mlpack::ann::ParametersVisitor(parameters), model.Model()[0]);
	// Get the weights (first part of parameters)
	for (int i = 0; i < L0.in_size; i++) {
		for (int j = 0; j < L0.out_size; j++) {
			L0.weights[i][j] = parameters[L0.out_size * i + j];
		}

	}
	// Get the biases (stored last in parameters)
	for (int i = 0; i < L0.out_size; i++) {
		L0.biases[i] = parameters[L0.in_size * L0.out_size + i];
	}

	// L1
	boost::apply_visitor(mlpack::ann::ParametersVisitor(parameters), model.Model()[1]);
	for (int i = 0; i < L1.in_size; i++) {
		for (int j = 0; j < L1.out_size; j++) {
			L1.weights[i][j] = parameters[L1.out_size * i + j];
		}
	}
	// Get the biases (stored last in parameters)
	for (int i = 0; i < L1.out_size; i++) {
		L1.biases[i] = parameters[L1.in_size * L1.out_size + i];
	}

	// L2
	boost::apply_visitor(mlpack::ann::ParametersVisitor(parameters), model.Model()[2]);
	for (int i = 0; i < L2.in_size; i++) {
		for (int j = 0; j < L2.out_size; j++) {
			L2.weights[i][j] = parameters[L2.out_size * i + j];
		}
	}// Get the biases (stored last in parameters)
	for (int i = 0; i < L2.out_size; i++) {
		L2.biases[i] = parameters[L2.in_size * L2.out_size + i];
	}

	// L3
	boost::apply_visitor(mlpack::ann::ParametersVisitor(parameters), model.Model()[3]);
	for (int i = 0; i < L3.in_size; i++) {
		L3.weights[i][0] = parameters[i];
	}
	L3.biases[0] = parameters[L3.in_size];
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
