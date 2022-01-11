#ifndef MLPACK_METHODS_ANN_LOSS_FUNCTION_MEAN_ABS_ERROR_IMPL_HPP
#define MLPACK_METHODS_ANN_LOSS_FUNCTION_MEAN_ABS_ERROR_IMPL_HPP

 // In case it hasn't yet been included.
#include "MeanAbsError.hpp"

namespace mlpack {
    namespace ann /** Artificial Neural Network. */ {

        template<typename InputDataType, typename OutputDataType>
        MeanAbsError<InputDataType, OutputDataType>::MeanAbsError()
        {
            // Nothing to do here.
        }

        template<typename InputDataType, typename OutputDataType>
        template<typename InputType, typename TargetType>
        typename InputType::elem_type
            MeanAbsError<InputDataType, OutputDataType>::Forward(
                const InputType& input,
                const TargetType& target)
        {
            return arma::accu(arma::abs(input - target)) / target.n_cols;
        }

        template<typename InputDataType, typename OutputDataType>
        template<typename InputType, typename TargetType, typename OutputType>
        void MeanAbsError<InputDataType, OutputDataType>::Backward(
            const InputType& input,
            const TargetType& target,
            OutputType& output)
        {
            OutputType one;
            one.ones(arma::size(input));

            // 1/n if target < input, -1/n else
            output = (OutputType)((target > input) * -2 + one) / target.n_cols;
        }

        template<typename InputDataType, typename OutputDataType>
        template<typename Archive>
        void MeanAbsError<InputDataType, OutputDataType>::serialize(
            Archive& /* ar */,
            const unsigned int /* version */)
        {
            // Nothing to do here.
        }

    } // namespace ann
} // namespace mlpack

#endif

