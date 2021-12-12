#include "LinearSplit.h"

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
            weights.set_size(outSize * inSize + outSize, 1);
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
            weight = arma::mat(weights.memptr(), outSize, inSize, false, false);
            bias = arma::mat(weights.memptr() + weight.n_elem,
                outSize, 1, false, false);
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            template<typename eT>
        void LinearSplit<InputDataType, OutputDataType, RegularizerType>::Forward(
            const arma::Mat<eT>& input, arma::Mat<eT>& output)
        {
            output = weight * input;
            output.each_col() += bias;
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            template<typename eT>
        void LinearSplit<InputDataType, OutputDataType, RegularizerType>::Backward(
            const arma::Mat<eT>& /* input */, const arma::Mat<eT>& gy, arma::Mat<eT>& g)
        {
            g = weight.t() * gy;
        }

        template<typename InputDataType, typename OutputDataType,
            typename RegularizerType>
            template<typename eT>
        void LinearSplit<InputDataType, OutputDataType, RegularizerType>::Gradient(
            const arma::Mat<eT>& input,
            const arma::Mat<eT>& error,
            arma::Mat<eT>& gradient)
        {
            gradient.submat(0, 0, weight.n_elem - 1, 0) = arma::vectorise(
                error * input.t());
            gradient.submat(weight.n_elem, 0, gradient.n_elem - 1, 0) =
                arma::sum(error, 1);
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

