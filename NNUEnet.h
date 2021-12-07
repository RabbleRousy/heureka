#pragma once

#include <mlpack/core.hpp>
#include <mlpack/methods/ann/ffn.hpp>
#include <mlpack/methods/ann/loss_functions/mean_squared_error.hpp>

// 2*FeatureSet[N]->M*2->K->1
// 2*HalfKP[40960]->256x2->32->32->1
const int N = 40960;
const int M = 256;
const int K = 32;

namespace mlpack {
	namespace ann {
		class NNUEnet : public FFN<MeanSquaredError<>> {
		public:
			// Access base class' overloaded methods
			using FFN<MeanSquaredError<>>::Forward;
			NNUEnet();
			void Forward(arma::mat inputs, arma::mat& results);
		};
	}
}
