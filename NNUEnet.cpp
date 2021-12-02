#include "NNUEnet.h"

mlpack::ann::NNUEnet::NNUEnet() {
	// L0
	Add<Linear<> >(N, M);
	Add<CReLU<>>();
	// L1
	Add<Linear<> >(2 * M, K);
	Add<CReLU<> >();
	// L2
	Add<Linear<> >(K, K);
	Add<CReLU<> >();
	// L3
	Add<Linear<> >(K, 1);
	Add<CReLU<> >();
}

// Custom forward pass through the network
void mlpack::ann::NNUEnet::Forward(arma::mat inputs, arma::mat& results) {

	// Split the input features in 2
	const arma::mat whiteFeatures(inputs.submat(0, 0, N - 1, 0));
	arma::mat blackFeatures(inputs.submat(N, 0, 2 * N - 1, 0));

	// Pass them through L0 separately
	arma::mat L0out_white;
	Forward(whiteFeatures, L0out_white, 0, 1);
	arma::mat L0out_black;
	FFN::Forward(blackFeatures, L0out_black, 0, 1);

	// Perform the rest of the regular forward pass 
	FFN::Forward(arma::join_cols(L0out_white, L0out_black), results, 2, 7);
}
