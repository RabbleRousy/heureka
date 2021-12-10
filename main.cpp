#include <mlpack/core.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>

int main() {
	const int N = 40960;
	const int M = 256;
	const int K = 32;

	arma::sp_mat sparseMatrix;
	// Only 50 training examples, as I haven't converted more to the correct format
	sparseMatrix.load("trainData.csv", arma::coord_ascii);
	sparseMatrix = sparseMatrix.t();
	// Not sure if I could continue to use sp_mat, would be great
	arma::mat data = (arma::mat)sparseMatrix.submat(0, 0, sparseMatrix.n_rows - 2, sparseMatrix.n_cols - 1);

	// Cut the labels from the last row of the trainingData
	arma::mat labels = (arma::mat)sparseMatrix.submat(sparseMatrix.n_rows - 1, 0, sparseMatrix.n_rows - 1, sparseMatrix.n_cols - 1);

	mlpack::ann::FFN<mlpack::ann::MeanSquaredError<>> network;

	mlpack::ann::Sequential<> firstHalf(2 * N, M);
	firstHalf.Add<mlpack::ann::Subview<>>(1, 0, N - 1, 0, 0);
	firstHalf.Add<mlpack::ann::Linear<>>(2 * N, M);
	firstHalf.Add<mlpack::ann::ReLULayer<>>();

	mlpack::ann::Sequential<> secondHalf(2 * N, M);
	secondHalf.Add<mlpack::ann::Subview<>>(1, N, 2 * N - 1, 0, 0);
	secondHalf.Add<mlpack::ann::Linear<>>(2 * N, M);
	secondHalf.Add<mlpack::ann::ReLULayer<>>();

	mlpack::ann::Concat<> concat(2 * N, 2 * M);
	concat.Add<mlpack::ann::Sequential<>>(firstHalf);
	concat.Add<mlpack::ann::Sequential<>>(secondHalf);

	// Add the concatenation to the network
	network.Add<mlpack::ann::Concat<>>(concat);
	// L1
	network.Add<mlpack::ann::Linear<> >(2 * M, K);
	network.Add<mlpack::ann::ReLULayer<> >();
	// L2
	network.Add<mlpack::ann::Linear<> >(K, K);
	network.Add<mlpack::ann::ReLULayer<> >();
	// L3
	network.Add<mlpack::ann::Linear<> >(K, 1);

	network.Train(data, labels);
	return 0;
}
