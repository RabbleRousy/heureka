#ifndef ENSMALLEN_CALLBACKS_PRINT_VAL_LOSS_HPP
#define ENSMALLEN_CALLBACKS_PRINT_VAL_LOSS_HPP

#include "NNUE.h"
#include <iostream>
#include <mlpack/methods/ann/ffn.hpp>

namespace ens {

    /**
     * Print validation loss function, based on the PrintLoss callback function.
     */
    class PrintValidationLoss
    {
    private:
        //! The output stream that all data is to be sent to; example: std::cout.
        std::ostream& output;

        mlpack::ann::FFN<NNUE::lossFunction>& network;

        arma::mat& validationData, validationLabels;

    public:
        /**
         * Set up the print loss callback class with the width and output stream.
         *
         * @param ostream Ostream which receives output from this object.
         */
        PrintValidationLoss(mlpack::ann::FFN<NNUE::lossFunction>& net, arma::mat& valData, arma::mat& valLabels, std::ostream& output = arma::get_cout_stream())
            : network(net), output(output), validationData(valData), validationLabels(valLabels)
        { /* Nothing to do here. */
        }

        /**
         * Callback function called at the end of a pass over the data.
         *
         * @param optimizer The optimizer used to update the function.
         * @param function Function to optimize.
         * @param coordinates Starting point.
         * @param epoch The index of the current epoch.
         * @param objective Objective value of the current point.
         */
        template<typename OptimizerType, typename FunctionType, typename MatType>
        void EndEpoch(OptimizerType& /* optimizer */,
            FunctionType& /* function */,
            const MatType& /* coordinates */,
            const size_t /* epoch */,
            const double objective)
        {
            // Predict the validation data
            arma::mat predictions;
            network.Predict(validationData, predictions);

            // TODO: ABS statt SQUARE
            // Calculate the mean squared error
            double loss = arma::accu(arma::square(predictions - validationLabels)) / predictions.n_cols;

            output << loss << std::endl;
        }
    };

} // namespace ens

#endif
