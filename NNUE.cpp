#include "NNUE.h"
#include "ValidationLoss.hpp"
#include "Board.h"

NNUE::NNUE() {
}

NNUE::NNUE(std::string modelPath) {
	loadModel(modelPath);
}

void NNUE::recalculateAccumulator(const std::vector<int> &activeFeatures, bool white) {

	if (white) 
		DEBUG_COUT("White accumulator recalculated from scratch.\n");
	else
		DEBUG_COUT("Black accumulator recalculated from scratch.\n");

	// Copy L1's bias
	for (int i = 0; i < M; i++) {
		accumulator[white][i] = L1.biases[i];
	}

	// Add the weights for active feature's column
	for (int a : activeFeatures) {
		for (int i = 0; i < M; i++) {
			accumulator[white][i] += L1.weights[a][i];
		}
	}
}

void NNUE::updateAccumulator(const std::vector<int>& removedFeatures, const std::vector<int>& addedFeatures, bool white) {
	if (white)
		DEBUG_COUT("White accumulator updated incrementally.\n");
	else
		DEBUG_COUT("Black accumulator updated incrementally.\n");

	// Subtract weights of removed Features
	for (int r : removedFeatures) {
		for (int i = 0; i < M; i++) {
			accumulator[white][i] -= L1.weights[r][i];
		}
	}
	// Add weights of added features
	for (int a : addedFeatures) {
		for (int i = 0; i < M; i++) {
			accumulator[white][i] += L1.weights[a][i];
		}
	}
}

void NNUE::crelu(int size, const float* input, float* output) {
	for (int i = 0; i < size; i++) {
		// Clip value between 0 and 1
		output[i] = std::min(std::max(0.0f, input[i]), 1.0f);
	}
}

float NNUE::evaluate(bool whiteToMove) {
	float input[2 * M];
	// Get the accumulators as one vector in the right order
	for (int i = 0; i < M; i++) {
		input[i] = accumulator[whiteToMove][i];
		input[i + M] = accumulator[!whiteToMove][i];
	}

	float output[2 * M];
	// Activation function for accumulator results (first hidden layer)
	crelu(2 * M, input, output);

	// second hidden layer (write output into old input buffer)
	linear<M * 2, K>(L2, output, input);
	crelu(K, input, output);

	// third hidden layer
	linear<K, K>(L3, output, input);
	crelu(K, input, output);

	// Output layer
	linear<K, 1>(L4, output, input);

	return input[0];
}

void NNUE::train(bool newNet, std::string modelPath, std::string dataPath, std::string valPath, double stepSize, int batchSize, int maxIterations, std::string predPath) {
	arma::sp_mat sparseMatrix;
	sparseMatrix.load(dataPath, arma::coord_ascii);
	arma::mat trainData = (arma::mat) sparseMatrix.t();
	
	// Shuffle data
	/*srand(time(NULL));
	for (int i = 0; i < trainData.n_cols; i++) {
		// Swap 2 random columns
		trainData.swap_cols(rand() % trainData.n_cols, rand() % trainData.n_cols);
	}*/

	// Cut the trainLabels from the last row of the trainingData
	arma::mat trainLabels = trainData.submat(trainData.n_rows - 1, 0, trainData.n_rows - 1, trainData.n_cols - 1);
	trainData = trainData.submat(0, 0, trainData.n_rows - 2, trainData.n_cols - 1);

	// Load validation data
	sparseMatrix.load(valPath, arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();
	arma::mat validationData = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);
	arma::mat validationLabels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);

	mlpack::ann::FFN<lossFunction> network;

	if (newNet) {
		// L1
		network.Add<mlpack::ann::LinearBitSplit<> >(2 * N, 2 * M);
		network.Add<mlpack::ann::ClippedReLULayer<>>();
		// L2									 
		network.Add<mlpack::ann::Linear<> >(2 * M, K);
		network.Add<mlpack::ann::ClippedReLULayer<>>();
		// L3									 
		network.Add<mlpack::ann::Linear<> >(K, K);
		network.Add<mlpack::ann::ClippedReLULayer<>>();
		// L4
		network.Add<mlpack::ann::Linear<> >(K, 1);
	}
	else {
		mlpack::data::Load(modelPath + "\\net.bin", "network", network);
		// Create backup
		mlpack::data::Save(modelPath + "\\net_backup.bin", "network", network, false);
	}
	
	// Stochastic Gradient Descent
	//ens::StandardSGD optimizer(stepSize, batchSize, maxIterations, -1);
	//ens::SPALeRASGD<> optimizer(stepSize, batchSize, maxIterations, -1);
	ens::Adam optimizer(stepSize, batchSize, 0.9, 0.9999999999, 1e-08, maxIterations, -1);

	// Open log streams
	std::ofstream lossOutput, valLossOutput, reportOutput, dataOutput;
	lossOutput.open(modelPath + "\\loss.txt", std::ios_base::app);
	valLossOutput.open(modelPath + "\\validationLoss.txt", std::ios_base::app);
	reportOutput.open(modelPath + "\\report.txt", std::ios_base::app); 

	// If there is no path specified for the predicition results, just put them in the model folder
	if (predPath == "") predPath = modelPath;

	// TRAIN THE MODEL
	network.Train(trainData, trainLabels, optimizer,
		/*Callbacks*/
		ens::ProgressBar(), /*ens::Report(0.1, reportOutput, 1),*/ ens::PrintLoss(lossOutput),
		ens::ValidationLoss(network, validationData, validationLabels, predPath, 1, valLossOutput, true, 10));

	mlpack::data::Save(modelPath + "\\net.bin", "network", network, false);

	lossOutput.close();
	valLossOutput.close();
	reportOutput.close();
}

void NNUE::train(bool newNet, std::string modelPath, double stepSize, int batchSize, const TrainSession& session) {

	std::cout << "Starting automized training session with:\n"
		<< '\t' << session.dataPerTraining << " samples per training\n"
		<< "\tSplit into " << session.dataChunks << " chunks in ranges of size " << session.chunkSize << '\n'
		<< '\t' << session.shift << " sample shift after each training\n"
		<< '\t' << session.epochsPerTraining << " epochs per training\n"
		<< '\t' << session.trainings << " total trainings.\n\n";

	if (newNet) {
		// Create the required subfolders for training
		bool check = _mkdir((modelPath + "\\trainData").c_str());
		if (!check) {
			std::cout << "Output directory for formatted training data created.\n";
		}
		else {
			std::cout << "Failed to create directory. Aborting...\n";
			return;
		}
	}

	// File for saving the data boundaries
	std::ofstream dataFile(modelPath + "\\trainData\\usedData.csv", std::ios_base::app);

	for (int i = 0; i < session.trainings; i++) {
		// Get and format the training data
		int dataMinIndex, dataMaxIndex;
		arma::sp_mat matrixLoader, trainData;

		std::cout << "\n\nFormatting data for training #" << i + 1 << " ......... ";
		for (int j = 0; j < session.dataChunks; j++) {
			int from = session.offset + j * session.chunkSize + i * session.shift;
			if (!j) dataMinIndex = from;
			int to = session.offset + j * session.chunkSize + i * session.shift + session.dataPerTraining / session.dataChunks;
			if (j == session.dataChunks - 1) dataMaxIndex = to;

			std::string outPath = modelPath + "\\trainData\\" + std::to_string(j+1) + "_1.csv";
			std::cout << "\nData indices: " << from << " - " << to << '\n';
			formatDataset(session.trainDataPath, outPath, from, to);
			std::cout << "Formatting finished.\n";

			// Immediately load and concatenate the formatted matrix
			matrixLoader.load(outPath, arma::coord_ascii);
			trainData = arma::join_cols(trainData, matrixLoader);
		}
		// Save the concatenated matrix
		trainData.save(modelPath + "\\trainData\\concat.csv", arma::coord_ascii);

		// Save the data boundaries
		dataFile << dataMinIndex << "," << dataMaxIndex << "\n";

		// Create a new folder for the predictions to be saved into
		std::string predictionsPath = modelPath + "\\predictionsFromEpoch" + std::to_string(i);
		_mkdir(predictionsPath.c_str());

		// Train
		std::cout << "Starting training #" << i + 1 << "...\n";
		train((i == 0) && newNet, modelPath, modelPath + "\\trainData\\concat.csv", session.valDataPath, stepSize, batchSize, session.dataPerTraining * session.epochsPerTraining, predictionsPath);
		std::cout << "\nTraining finished.\n\n";
	}
	dataFile.close();
}

void NNUE::formatDataset(std::string inPath, std::string outPath, int from, int to) {
	std::string line;
	std::string fen;
	std::ifstream input(inPath);
	std::ofstream output(outPath);

	// Create a board to help with fen reading
	Board board;
	// First line is header
	std::getline(input, line);

	unsigned long long row = 0;

	for (int i = 0; i < from; i++) {
		std::getline(input, line);
	}

	while (std::getline(input, line) && row < (to - from)) {
		/*
		* FOR SKIPPING RANDOM SAMPLES
		* 
		float limit = RAND_MAX / 10.0f;
		std::cout << "Limit is " << limit << '\n';
		int x = rand();
		std::cout << "Rolled: " << x << " >> ";
		if (x > limit) {
			std::cout << "Skipped.\n";
			continue;
		}
		std::cout << "Accepted.\n";
		*/

		// label and value are comma-separated
		fen = line.substr(0, line.find(','));
		board.readPosFromFEN(fen);
		//board.print();
		std::string e = line.substr(line.find(',')+1); 
		int evalCP;
		float evalWDL;

		// Ignore samples were there is a forced mate
		if (e.find('#') == std::string::npos) {
			evalCP = std::stoi(e);
			if (!board.gameState.whiteToMove()) {
				evalCP = -evalCP;
			}
			// Transform evaluation from centipawns to win/draw/loss (0/0.5/1.0)
			evalWDL = utils::math::sigmoid(evalCP, 0, 1.0f / 410.0f);


			// Coordinate list format for sparse matrices:
			// <row> <column> <nonzero-value>
			std::string coordinateList = getHalfKPcoordinateList(row);
			// Append value
			coordinateList += std::to_string(row) + " 1300 " + std::to_string(evalWDL) + '\n';

			output << coordinateList;
			row++;
		}
	}

	input.close();
	output.close();
}

void NNUE::predictTest(std::string modelPath, std::string testdataPath, std::string outputName) {
	arma::sp_mat sparseMatrix;
	sparseMatrix.load(testdataPath, arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();

	arma::mat data = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);
	// Get the labels from the last row of the data
	arma::mat labels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);

	arma::mat prediction;
	mlpack::ann::FFN<lossFunction> network;
	mlpack::data::Load(modelPath + "\\net.bin", "net", network);

	// Output log
	std::ofstream predictOut(modelPath + outputName);
	
	double errorSum = 0;

	for (int i = 0; i < data.n_cols; i++) {
		auto column = data.col(i);
		network.Predict(column, prediction);
		double pred = prediction[0];
		double label = labels[i];
		double error = std::abs(pred - label);
		errorSum += error;
		std::cout << "Prediction for #" << i << " : " << pred << " (Label: " << labels[i] << ", off by " << std::abs(pred - labels[i]) << ")\n";
		// Write it all to the log file
		predictOut << pred << ',' << label << ',' << error << '\n';
	}
	std::cout << "AVERAGE ERROR: " << errorSum / data.n_cols;
}

unsigned int NNUE::getHalfPieceIndex(short square, short pieceType, short our) {
	return 65 * (10 * square + 5 * our + pieceType - 2);
}

unsigned int NNUE::getHalfKPindex(short perspective, short pieceType, short pieceColor, short square, short kingSquare) {
	if (perspective == Piece::BLACK) {
		// If it's black's perspective, flip board
		kingSquare ^= 56;
		square ^= 56;
	}
	// Half P feature from 0 to 65 * 639
	unsigned int p = getHalfPieceIndex(square, pieceType, pieceColor == perspective);
	return p + kingSquare + 1;
}

std::string NNUE::getHalfKPcoordinateList(unsigned long long row) {
	std::string cl;

	// Perspective of side to move (first HalfKP)
	unsigned short kingSquare = Board::gameState.whiteToMove() ? Board::whiteKingPos : Board::blackKingPos;
	unsigned short pieceSquare;

	// Each word holds 64 features
	unsigned long long* words = new unsigned long long[(2 * N) / 64];
	memset(words, 0ull, sizeof(words) * ((2 * N) / 64));

	for (short piece = Piece::PAWN; piece <= Piece::QUEEN; piece++) {
		for (short color = Piece::WHITE; color <= Piece::BLACK; color+=8) {

			bitboard pieces = Board::bb.getBitboard(piece | color);
			Bitloop(pieces) {
				pieceSquare = getSquare(pieces);
				unsigned int halfKPindex = getHalfKPindex(Board::gameState.currentPlayer, piece, color, pieceSquare, kingSquare);
				words[int(halfKPindex / 64)] |= (1ull << halfKPindex % 64);

				if (!Board::gameState.whiteToMove()) {
					// First perspective is black, flip board
					pieceSquare ^= 56;
				}
				unsigned int halfPieceIndex = getHalfPieceIndex(pieceSquare, piece, color == Board::gameState.currentPlayer);
				words[int(halfPieceIndex / 64)] |= (1ull << halfPieceIndex % 64);
			}
		}
	}

	// Perspective of the other side (second HalfKP)
	kingSquare = Board::gameState.whiteToMove() ?  Board::blackKingPos : Board::whiteKingPos;

	for (short piece = Piece::PAWN; piece <= Piece::QUEEN; piece++) {
		for (short color = Piece::WHITE; color <= Piece::BLACK; color+=8) {
			bitboard pieces = Board::bb.getBitboard(piece | color);
			Bitloop(pieces) {
				pieceSquare = getSquare(pieces);
				unsigned int halfKPindex = 41600 + getHalfKPindex(Piece::getOppositeColor(Board::gameState.currentPlayer), piece, color, pieceSquare, kingSquare);
				words[int(halfKPindex / 64)] |= (1ull << halfKPindex % 64);

				if (Board::gameState.whiteToMove()) {
					// Second perspective is black, flip board
					pieceSquare ^= 56;
				}
				unsigned int halfPieceIndex = 41600 + getHalfPieceIndex(pieceSquare, piece, color == Board::gameState.currentPlayer);
				words[int(halfPieceIndex / 64)] |= (1ull << halfPieceIndex % 64);
			}
		}
	}

	// Write the words into the coordinate list
	for (int i = 0; i < (2 * N) / 64; i++) {
		unsigned long long wordValue = words[i];
		if (wordValue == 0ull)
			continue;
		cl += std::to_string(row) + ' ' + std::to_string(i) + ' ' + std::to_string(words[i]) + '\n';
	}
	delete[] words;
	
	return cl;
}

void NNUE::loadModel(std::string path) {
	mlpack::ann::FFN<> model;
	mlpack::data::Load(path, "model", model);
	
	// L1
	arma::mat parameters;
	boost::apply_visitor(mlpack::ann::ParametersVisitor(parameters), model.Model()[0]);
	// Get the weights (first part of parameters)
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
	}
	// Get the biases (stored last in parameters)
	for (int i = 0; i < L2.out_size; i++) {
		L2.biases[i] = parameters[L2.in_size * L2.out_size + i];
	}

	// L3
	boost::apply_visitor(mlpack::ann::ParametersVisitor(parameters), model.Model()[4]);
	for (int i = 0; i < L3.in_size; i++) {
		for (int j = 0; j < L3.out_size; j++) {
			L3.weights[i][j] = parameters[L3.out_size * i + j];
		}
	}// Get the biases (stored last in parameters)
	for (int i = 0; i < L3.out_size; i++) {
		L3.biases[i] = parameters[L3.in_size * L3.out_size + i];
	}

	// L4
	boost::apply_visitor(mlpack::ann::ParametersVisitor(parameters), model.Model()[6]);
	for (int i = 0; i < L4.in_size; i++) {
		L4.weights[i][0] = parameters[i];
	}
	L4.biases[0] = parameters[L4.in_size];
}

void NNUE::printHalfKPindeces() {
	using namespace std;
	ofstream file("virtual features indices list.txt");

	for (int pieceSquare = 0; pieceSquare < 64; pieceSquare++) {
		for (int our = 0; our < 2; our++) {
			for (short pieceType = Piece::PAWN; pieceType <= Piece::QUEEN; pieceType++) {
				for (short kingSquare = 0; kingSquare < 64; kingSquare++) {
					if (kingSquare == 0)
						file << "\n\nP Index of (piece on " << Board::getSquareName(pieceSquare) << (our ? ", our, " : ", their, ") << Piece::name(pieceType) << "): "
						<< getHalfPieceIndex(pieceSquare, pieceType, our);

					file << "\nHalfKP Index of (king on " << Board::getSquareName(kingSquare) << ", piece on " << Board::getSquareName(pieceSquare)
						<< (our ? ", our, " : ", their, ") << Piece::name(pieceType) << "): "
						<< getHalfKPindex(Piece::WHITE, pieceType, our ? Piece::WHITE : Piece::BLACK, pieceSquare, kingSquare);
				}
			}
		}
	}
	file.close();
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
