#include "NNUE.h"

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
	sparseMatrix = sparseMatrix.t();
	
	// Shuffle data
	srand(time(NULL));
	for (int i = 0; i < sparseMatrix.n_cols; i++) {
		// Swap 2 random columns
		sparseMatrix.swap_cols(rand() % sparseMatrix.n_cols, rand() % sparseMatrix.n_cols);
	}


	arma::mat trainData = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);

	// Cut the trainLabels from the last row of the trainingData
	arma::mat trainLabels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);

	// Load validation data
	sparseMatrix.load(valPath, arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();
	arma::mat validationData = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);
	arma::mat validationLabels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);


	/*sparseMatrix.load("C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\random_evalsFormatted10k.csv", arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();
	arma::mat validationData = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);
	arma::mat validationLabels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);*/

	mlpack::ann::FFN<mlpack::ann::MeanSquaredError<>> network;

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
	double stepSize, int batchSize, int maxIterations, std::string valPath) {
	arma::sp_mat sparseMatrix;
	arma::mat trainData;

	for (int i = 0; i < 10; i++) {
		if (batchCounts[i] == 0) continue;

		for (int j = 0; j < batchCounts[i]; j++) {
			std::string path = "C:\\Users\\simon\\Documents\\Hochschule\\Schachengine\\TrainingSets\\random_evals\\"
				+ std::to_string(i + 1) + '_' + std::to_string(j+1) + ".csv";
			sparseMatrix.load(path, arma::coord_ascii);
			sparseMatrix = sparseMatrix.t();

			trainData = arma::join_rows(trainData, (arma::mat)sparseMatrix);
		}
	}

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

	mlpack::ann::FFN<mlpack::ann::MeanSquaredError<>> network;

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
			coordinateList += std::to_string(row) + " 81920 " + std::to_string(eval) + '\n';

			output << coordinateList;
			row++;
		}
	}

	input.close();
	output.close();
}

void NNUE::predictTest(std::string modelPath, std::string testdataPath) {
	arma::sp_mat sparseMatrix;
	sparseMatrix.load(testdataPath, arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();

	arma::mat data = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);
	// Get the labels from the last row of the data
	arma::mat labels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);

	arma::mat prediction;
	mlpack::ann::FFN<mlpack::ann::MeanSquaredError<>> network;
	mlpack::data::Load(modelPath + "\\net.bin", "net", network);

	// Output log
	std::ofstream predictOut(modelPath + "\\predictions.csv");
	
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

unsigned int NNUE::getHalfKPindex(short perspective, short pieceType, short pieceColor, short square, short kingSquare) {
	if (perspective == Piece::BLACK) {
		// If it's black's perspective, flip board
		kingSquare ^= 56;
		square ^= 56;
	}
	return 640 * kingSquare + 10 * square + 5 * (pieceColor == perspective) + Piece::getType(pieceType) - 2;
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
				unsigned int index = 40960 + getHalfKPindex(Piece::getOppositeColor(board->currentPlayer), piece, color, pieceSquare, kingSquare);
				cl += std::to_string(row) + ' ' + std::to_string(index) + " 1.0\n";
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
