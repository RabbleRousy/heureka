#ifndef MLPACK_METHODS_ANN_LAYER_LINEAR_SPLIT_IMPL_HPP
#define MLPACK_METHODS_ANN_LAYER_LINEAR_SPLIT_IMPL_HPP

// In case it hasn't yet been included.
#include "LinearSplit.hpp"

namespace mlpack {
    namespace ann /** Artificial Neural Network. */ {

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            LinearSplit<InputDataType, OutputDataType, RegularizerType>::LinearSplit() :
            inSize(0),
            outSize(0)
        {
            // Nothing to do here.
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            LinearSplit<InputDataType, OutputDataType, RegularizerType>::LinearSplit(
                const size_t inSize,
                const size_t outSize,
                RegularizerType regularizer) :
            inSize(inSize),
            outSize(outSize),
            regularizer(regularizer)
        {
            // We only have half the amount of weights and biases, as we use the same matrix twice
            weights.set_size((outSize / 2) * (inSize / 2) + (outSize / 2), 1);
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            LinearSplit<InputDataType, OutputDataType, RegularizerType>::LinearSplit(
                const LinearSplit& layer) :
            inSize(layer.inSize),
            outSize(layer.outSize),
            weights(layer.weights),
            regularizer(layer.regularizer)
        {
            // Nothing to do here.
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            LinearSplit<InputDataType, OutputDataType, RegularizerType>::LinearSplit(
                LinearSplit&& layer) :
            inSize(0),
            outSize(0),
            weights(std::move(layer.weights)),
            regularizer(std::move(layer.regularizer))
        {
            // Nothing to do here.
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            LinearSplit<InputDataType, OutputDataType, RegularizerType>&
            LinearSplit<InputDataType, OutputDataType, RegularizerType>::
            operator=(const LinearSplit& layer)
        {
            if (this != &layer)
            {
                inSize = layer.inSize;
                outSize = layer.outSize;
                weights = layer.weights;
                regularizer = layer.regularizer;
            }
            return *this;
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            LinearSplit<InputDataType, OutputDataType, RegularizerType>&
            LinearSplit<InputDataType, OutputDataType, RegularizerType>::
            operator=(LinearSplit&& layer)
        {
            if (this != &layer)
            {
                inSize = layer.inSize;
                outSize = layer.outSize;
                weights = std::move(layer.weights);
                regularizer = std::move(layer.regularizer);
            }
            return *this;
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            void LinearSplit<InputDataType, OutputDataType, RegularizerType>::Reset()
        {
            weight = arma::mat(weights.memptr(), outSize / 2, inSize / 2, false, false);
            bias = arma::mat(weights.memptr() + weight.n_elem,
                outSize / 2, 1, false, false);
        }

        // Custom Forward pass splits the input in half and applies the weights and biases separately before joining the two halves back together
        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            template<typename eT>
        void LinearSplit<InputDataType, OutputDataType, RegularizerType>::Forward(
            const arma::Mat<eT>& input, arma::Mat<eT>& output)
        {
            arma::mat i_firstHalf = input.submat(0, 0, inSize / 2 - 1, input.n_cols - 1);
            arma::mat i_secondHalf = input.submat(inSize / 2, 0, inSize - 1, input.n_cols - 1);
            arma::mat o_firstHalf = weight * i_firstHalf;
            o_firstHalf.each_col() += bias;
            arma::mat o_secondHalf = weight * i_secondHalf;
            o_secondHalf.each_col() += bias;
            output = arma::join_cols(o_firstHalf, o_secondHalf);
        }

        // Backward pass also calculates two halves of gradient vector separately
        // Never called in my usecase because LinearSplit is first layer
        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            template<typename eT>
        void LinearSplit<InputDataType, OutputDataType, RegularizerType>::Backward(
            const arma::Mat<eT>& /* input */, const arma::Mat<eT>& gy, arma::Mat<eT>& g)
        {
            arma::mat error_firstHalf = gy.submat(0, 0, outSize / 2 - 1, gy.n_cols - 1);
            arma::mat error_secondHalf = gy.submat(outSize / 2, 0, outSize - 1, gy.n_cols - 1);
            g = arma::join_cols(weight.t() * error_firstHalf, weight.t() * error_secondHalf);
        }

        // Evaluate twice, using one Gradient from the first half, and another one from the second
        // Half of gradient matrix stays unused at is has double size of our actual weights
        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            template<typename eT>
        void LinearSplit<InputDataType, OutputDataType, RegularizerType>::Gradient(
            const arma::Mat<eT>& input,
            const arma::Mat<eT>& error,
            arma::Mat<eT>& gradient)
        {
            arma::mat i_firstHalf = input.submat(0, 0, inSize / 2 - 1, input.n_cols - 1);
            arma::mat i_secondHalf = input.submat(inSize / 2, 0, inSize - 1, input.n_cols - 1);
            arma::mat error_firstHalf = error.submat(0, 0, outSize / 2 - 1, error.n_cols - 1);
            arma::mat error_secondHalf = error.submat(outSize / 2, 0, outSize - 1, error.n_cols - 1);

            gradient.submat(0, 0, weight.n_elem - 1, 0) = arma::vectorise(
                error_firstHalf * i_firstHalf.t());
            gradient.submat(weight.n_elem, 0, gradient.n_elem - 1, 0) =
                arma::sum(error_firstHalf, 1);
            regularizer.Evaluate(weights, gradient);

            gradient.submat(0, 0, weight.n_elem - 1, 0) = arma::vectorise(
                error_secondHalf * i_secondHalf.t());
            gradient.submat(weight.n_elem, 0, gradient.n_elem - 1, 0) =
                arma::sum(error_secondHalf, 1);
            regularizer.Evaluate(weights, gradient);
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            template<typename Archive>
        void LinearSplit<InputDataType, OutputDataType, RegularizerType>::serialize(
            Archive& ar, const unsigned int /* version */)
        {
            ar& BOOST_SERIALIZATION_NVP(inSize);
            ar& BOOST_SERIALIZATION_NVP(outSize);
            ar& BOOST_SERIALIZATION_NVP(weights);
        }

    } // namespace ann
} // namespace mlpack
#endif