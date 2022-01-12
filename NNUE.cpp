#include "NNUE.h"
#include "PrintValidationLoss.hpp"
#include "Board.h"

NNUE::NNUE() {
	//loadModel("C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainedNets\\CustomLayer\\CustomLayer1k.txt");
}

NNUE::NNUE(std::string modelPath) {
	//loadModel(modelPath);
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

void NNUE::relu(int size, const float* input, float* output) {
	for (int i = 0; i < size; i++) {
		// Clip value between 0 and 1
		output[i] = std::max(0.0f, input[i]);
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
	relu(2 * M, input, output);

	// First hidden layer (write output into old input buffer)
	linear<M * 2, K>(L1, output, input);
	relu(K, input, output);

	// Second hidden layer
	linear<K, K>(L2, output, input);
	relu(K, input, output);

	// Output layer
	linear<K, 1>(L3, output, input);

	return input[0];
}

void NNUE::train(bool newNet, std::string modelPath, std::string dataPath, std::string valPath, double stepSize, int batchSize, int maxIterations) {
	arma::sp_mat sparseMatrix;
	sparseMatrix.load(dataPath, arma::coord_ascii);
	arma::mat trainData = (arma::mat) sparseMatrix.t();
	
	// Shuffle data
	srand(time(NULL));
	for (int i = 0; i < trainData.n_cols; i++) {
		// Swap 2 random columns
		trainData.swap_cols(rand() % trainData.n_cols, rand() % trainData.n_cols);
	}


	trainData = trainData.submat(0, 0, trainData.n_rows - 2, trainData.n_cols - 1);

	// Cut the trainLabels from the last row of the trainingData
	arma::mat trainLabels = trainData.submat(trainData.n_rows - 1, 0, trainData.n_rows - 1, trainData.n_cols - 1);

	// Load validation data
	sparseMatrix.load(valPath, arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();
	arma::mat validationData = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);
	arma::mat validationLabels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);


	/*sparseMatrix.load("C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\random_evalsFormatted10k.csv", arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();
	arma::mat validationData = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);
	arma::mat validationLabels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);*/

	mlpack::ann::FFN<lossFunction> network;

	if (newNet) {
		// L0
		network.Add<mlpack::ann::LinearSplit<> >(2 * N, 2 * M);
		network.Add<mlpack::ann::ClippedReLULayer<>>();
		// L1									 
		network.Add<mlpack::ann::Linear<> >(2 * M, K);
		network.Add<mlpack::ann::ClippedReLULayer<>>();
		// L2									 
		network.Add<mlpack::ann::Linear<> >(K, K);
		network.Add<mlpack::ann::ClippedReLULayer<>>();
		// L3
		network.Add<mlpack::ann::Linear<> >(K, 1);
	}
	else {
		mlpack::data::Load(modelPath + "\\net.bin", "network", network);
		// Create backup
		mlpack::data::Save(modelPath + "\\net_backup.bin", "network", network, false);
	}
	

	/*for (int i = 0; i < 100; i++) {
		network.Train(trainData.submat(0, i * 32, trainData.n_rows - 1, (i+1) * 32 - 1),
					trainLabels.submat(0, i * 32, trainLabels.n_rows - 1, (i + 1) * 32 - 1),
					ens::GradientDescent::GradientDescent(),
					ens::ProgressBar(), ens::PrintLoss());

		arma::mat prediction;
		network.Predict(validationData, prediction);
		
		double meanSquaredError = arma::accu(arma::square(prediction - validationLabels)) / prediction.n_cols;
		std::cout << "MSE with validation set after epoch #" << i << " is " << meanSquaredError << ".\n\n";
	}*/
	
	// Stochastic Gradient Descent using Adam Optimizer
	ens::AMSGrad optimizer(stepSize, batchSize, 0.9, 0.999, 1e-8, 500000, -1);
	optimizer.MaxIterations() = maxIterations;

	// What I want (somehow not supported / compatible with ProgressBar() because of 'missing' batchsize)
	//ens::GradientDescent myOptimizer(0.01, 100000, 1e-3);
	//myOptimizer.MaxIterations() = 0;

	std::ofstream lossOutput, valLossOutput, reportOutput;
	lossOutput.open(modelPath + "\\loss.txt", std::ios_base::app);
	valLossOutput.open(modelPath + "\\validationLoss.txt", std::ios_base::app);
	reportOutput.open(modelPath + "\\report.txt", std::ios_base::app);


	network.Train(trainData, trainLabels, optimizer,
	/*Callbacks*/ens::ProgressBar(),
		ens::PrintLoss(lossOutput), ens::PrintValidationLoss(network, validationData, validationLabels, valLossOutput) );

	mlpack::data::Save(modelPath + "\\net.bin", "network", network, false);

	lossOutput.close();
	reportOutput.close();
}

void NNUE::train(bool newNet, std::string modelPath, std::array<int, 10> batchCounts,
	double stepSize, int batchSize, int maxIterations, std::string valPath, std::string batchesPath) {
	arma::sp_mat sparseMatrix;
	arma::sp_mat sp_trainData;

	for (int i = 0; i < 10; i++) {
		if (batchCounts[i] == 0) continue;

		for (int j = 0; j < batchCounts[i]; j++) {
			std::string path = batchesPath + std::to_string(i + 1) + '_' + std::to_string(j + 1) + ".csv";
			std::cout << "Loading traindata \"" << path << "\"...\n";
			sparseMatrix.load(path, arma::coord_ascii);

			sp_trainData = arma::join_cols(sp_trainData, sparseMatrix);
		}
	}

	arma::mat trainData = (arma::mat)sp_trainData.t();

	// Shuffle data
	srand(time(NULL));
	for (int i = 0; i < trainData.n_cols; i++) {
		// Swap 2 random columns
		trainData.swap_cols(rand() % trainData.n_cols, rand() % trainData.n_cols);
	}

	// Cut the trainLabels from the last row of the trainingData
	arma::mat trainLabels = (arma::mat)trainData.submat(trainData.n_rows - 1, 0, trainData.n_rows - 1, trainData.n_cols - 1);
	trainData = (arma::mat)trainData.submat(0, 0, trainData.n_rows - 2, trainData.n_cols - 1);

	// Load validation data
	sparseMatrix.load(valPath, arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();
	arma::mat validationData = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);
	arma::mat validationLabels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);

	mlpack::ann::FFN<lossFunction> network;

	if (newNet) {
		// L0
		network.Add<mlpack::ann::LinearSplit<> >(2 * N, 2 * M);
		network.Add<mlpack::ann::ClippedReLULayer<>>();
		// L1									 
		network.Add<mlpack::ann::Linear<> >(2 * M, K);
		network.Add<mlpack::ann::ClippedReLULayer<>>();
		// L2									 
		network.Add<mlpack::ann::Linear<> >(K, K);
		network.Add<mlpack::ann::ClippedReLULayer<>>();
		// L3
		network.Add<mlpack::ann::Linear<> >(K, 1);
	}
	else {
		mlpack::data::Load(modelPath + "\\net.bin", "network", network);
		// Create backup
		mlpack::data::Save(modelPath + "\\net_backup.bin", "network", network, false);
	}

	// Stochastic Gradient Descent using Adam Optimizer
	ens::AMSGrad optimizer(stepSize, batchSize, 0.9, 0.999, 1e-8, 500000, -1);
	optimizer.MaxIterations() = maxIterations;

	// Open log streams
	std::ofstream lossOutput, valLossOutput, reportOutput, dataOutput;
	lossOutput.open(modelPath + "\\loss.txt", std::ios_base::app);
	valLossOutput.open(modelPath + "\\validationLoss.txt", std::ios_base::app);
	reportOutput.open(modelPath + "\\report.txt", std::ios_base::app);
	dataOutput.open(modelPath + "\\usedData.csv", std::ios_base::app);

	// TRAIN THE MODEL
	network.Train(trainData, trainLabels, optimizer,
		/*Callbacks*/ens::ProgressBar(), ens::Report(0.1, reportOutput, 1),
		ens::PrintLoss(lossOutput), ens::PrintValidationLoss(network, validationData, validationLabels, valLossOutput));

	// Save the model
	mlpack::data::Save(modelPath + "\\net.bin", "network", network, false);

	// Save information on the data that was used
	dataOutput << int(maxIterations / trainData.n_cols);
	for (int i = 0; i < 10; i++) {
		dataOutput << ',' << batchCounts[i];
	}
	dataOutput << std::endl;

	// Close all streams
	dataOutput.close();
	valLossOutput.close();
	lossOutput.close();
	reportOutput.close();
}

void NNUE::autoTrain() {
	const int offset = 110000;
	const int dataPerTraining = 20000;
	const int dataChunks = 10;
	const int chunkSize = dataPerTraining * 0.55;
	const int epochsPerTraining = 10;
	const int shift = dataPerTraining / dataChunks * 0.5;
	const int trainings = 10;

	std::cout << "Starting automized training with:\n"
		<< '\t' << dataPerTraining << " samples per training\n"
		<< '\t' << epochsPerTraining << " epochs per training\n"
		<< '\t' << shift << " sample shift after each training\n"
		<< '\t' << trainings << " total trainings.\n\n";

	const std::string path = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainedNets\\AutoTrain3";
	bool check = _mkdir((path + "\\trainData").c_str());
	if (!check) {
		std::cout << "Output directory for formatted training data created.\n";
	}
	else {
		std::cout << "Failed to create directory. Aborting...\n";
		return;
	}

	const std::string validationDataPath = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\validation_rdm_upper500k.csv";

	for (int i = 0; i < trainings; i++) {
		// Get and format the training data
		std::string inPath = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\random_evals.csv";

		for (int j = 0; j < dataChunks; j++) {
			int from = offset + j * chunkSize + i * shift;
			int to = offset + j * chunkSize + i * shift + dataPerTraining / dataChunks;
			std::string outPath = path + "\\trainData\\" + std::to_string(j+1) + "_1.csv";
			std::cout << "\n\nFormatting data for training #" << i + 1 << "......... ";
			formatDataset(inPath, outPath, from, to);
			std::cout << "Formatting finished.\n";
		}

		// Train
		std::cout << "Starting training #" << i + 1 << "...\n";
		std::array<int, 10> arr;
		arr.fill(1);
		train(i == 0, path, arr, 0.00001, 256, dataPerTraining * epochsPerTraining, validationDataPath, path + "\\trainData\\");
		std::cout << "\nTraining finished.\nExecuting prediction test.....\n";

		// Predict
		predictTest(path, "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\validation_rdm_upper500k.csv",
			"\\predictions" + std::to_string((i+1) * epochsPerTraining));
	}
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
		float eval;

		// Ignore samples were there is a forced mate
		if (e.find('#') == std::string::npos) {
			eval = std::stof(e);
			if (board.currentPlayer == Piece::BLACK) {
				eval = -eval;
			}
			// Transform evaluation from centipawns to win/draw/loss (0/0.5/1.0)
			eval = utils::math::sigmoid(eval, 0, 1.0f / 410.0f);


			// Coordinate list format for sparse matrices:
			// <row> <column> <nonzero-value>
			std::string coordinateList = getHalfKPcoordinateList(row, &board);
			// Append value
			coordinateList += std::to_string(row) + " 83200 " + std::to_string(eval) + '\n';

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

void NNUE::recalculateAccumulators(const Board* board) {

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

std::string NNUE::getHalfKPcoordinateList(unsigned long long row, Board* board) {
	std::string cl;
	bool whiteToMove = board->currentPlayer == Piece::WHITE;

	// Perspective of side to move (first HalfKP)
	unsigned short kingSquare = whiteToMove ? board->whiteKingPos : board->blackKingPos;
	unsigned short pieceSquare;

	for (short piece = Piece::PAWN; piece <= Piece::QUEEN; piece++) {
		for (short color = Piece::WHITE; color <= Piece::BLACK; color+=8) {

			bitboard pieces = board->bb.getBitboard(piece | color);
			Bitloop(pieces) {
				pieceSquare = getSquare(pieces);
				unsigned int halfKPindex = getHalfKPindex(board->currentPlayer, piece, color, pieceSquare, kingSquare);
				cl += std::to_string(row) + ' ' + std::to_string(halfKPindex) + " 1.0\n";
				if (!whiteToMove) {
					// First perspective is black, flip board
					pieceSquare ^= 56;
				}
				unsigned int halfPieceIndex = getHalfPieceIndex(pieceSquare, piece, color == board->currentPlayer);
				cl += std::to_string(row) + ' ' + std::to_string(halfPieceIndex) + " 1.0\n";
			}
		}
	}

	// Perspective of the other side (second HalfKP)
	kingSquare = whiteToMove ?  board->blackKingPos : board->whiteKingPos;

	for (short piece = Piece::PAWN; piece <= Piece::QUEEN; piece++) {
		for (short color = Piece::WHITE; color <= Piece::BLACK; color+=8) {
			bitboard pieces = board->bb.getBitboard(piece | color);
			Bitloop(pieces) {
				pieceSquare = getSquare(pieces);
				unsigned int halfKPindex = 41600 + getHalfKPindex(Piece::getOppositeColor(board->currentPlayer), piece, color, pieceSquare, kingSquare);
				cl += std::to_string(row) + ' ' + std::to_string(halfKPindex) + " 1.0\n";
				if (whiteToMove) {
					// Second perspective is black, flip board
					pieceSquare ^= 56;
				}
				unsigned int halfPieceIndex = 41600 + getHalfPieceIndex(pieceSquare, piece, color == board->currentPlayer);
				cl += std::to_string(row) + ' ' + std::to_string(halfPieceIndex) + " 1.0\n";
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
