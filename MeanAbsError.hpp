#ifndef MLPACK_METHODS_ANN_LOSS_FUNCTION_MEAN_ABS_ERROR_HPP
#define MLPACK_METHODS_ANN_LOSS_FUNCTION_MEAN_ABS_ERROR_HPP

#include <mlpack/prereqs.hpp>

namespace mlpack {
    namespace ann /** Artificial Neural Network. */ {

        /**
         * The mean absolute error performance function measures the network's
         * performance according to the mean of absolute errors.
         *
         * @tparam ActivationFunction Activation function used for the embedding layer.
         * @tparam InputDataType Type of the input data (arma::colvec, arma::mat,
         *         arma::sp_mat or arma::cube).
         * @tparam OutputDataType Type of the output data (arma::colvec, arma::mat,
         *         arma::sp_mat or arma::cube).
         */
        template <
            typename InputDataType = arma::mat,
            typename OutputDataType = arma::mat
        >
            class MeanAbsError
        {
        public:
            /**
             * Create the MeanAbsError object.
             */
            MeanAbsError();

            /**
             * Computes the mean squared error function.
             *
             * @param input Input data used for evaluating the specified function.
             * @param target The target vector.
             */
            template<typename InputType, typename TargetType>
            typename InputType::elem_type Forward(const InputType& input,
                const TargetType& target);

            /**
             * Ordinary feed backward pass of a neural network.
             *
             * @param input The propagated input activation.
             * @param target The target vector.
             * @param output The calculated error.
             */
            template<typename InputType, typename TargetType, typename OutputType>
            void Backward(const InputType& input,
                const TargetType& target,
                OutputType& output);

            //! Get the output parameter.
            OutputDataType& OutputParameter() const { return outputParameter; }
            //! Modify the output parameter.
            OutputDataType& OutputParameter() { return outputParameter; }

            /**
             * Serialize the layer
             */
            template<typename Archive>
            void serialize(Archive& ar, const unsigned int /* version */);

        private:
            //! Locally-stored output parameter object.
            OutputDataType outputParameter;
        }; // class MeanAbsError

    } // namespace ann
} // namespace mlpack

// Include implementation.
#include "MeanAbsError_impl.hpp"

#endif

