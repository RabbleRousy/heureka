#ifndef ENSMALLEN_CALLBACKS_VAL_LOSS_HPP
#define ENSMALLEN_CALLBACKS_VAL_LOSS_HPP

#include "NNUE.h"
#include <iostream>
#include <mlpack/methods/ann/ffn.hpp>

namespace ens {

    /**
     * Manage validation loss, based on the PrintLoss callback function.
     */
    class ValidationLoss
    {
    private:
        //! The output stream that all data is to be sent to; example: std::cout.
        std::ostream& output;

        std::string& predictionsPath;

        int predictionsInterval;

        mlpack::ann::FFN<NNUE::lossFunction>& network;

        arma::mat& validationData, validationLabels;

        bool earlyStopAtMinLoss;

        double bestLoss;

        int steps;

        unsigned int patience;

    public:
        /**
         * Set up the print loss callback class with the width and output stream.
         *
         * @param ostream Ostream which receives output from this object.
         */
        ValidationLoss(mlpack::ann::FFN<NNUE::lossFunction>& net, arma::mat& valData, arma::mat& valLabels, std::string& predictionFilesPath, int savePredictionsInterval,
            std::ostream& output = arma::get_cout_stream(), bool earlyStop = false, unsigned int pat = 10)
            : network(net), output(output), validationData(valData), validationLabels(valLabels), predictionsPath(predictionFilesPath),
            predictionsInterval(savePredictionsInterval), earlyStopAtMinLoss(earlyStop), patience(pat), steps(0), bestLoss(100000.0)
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
        bool EndEpoch(OptimizerType& /* optimizer */,
            FunctionType& /* function */,
            const MatType& /* coordinates */,
            const size_t epoch,
            const double objective)
        {
            // Predict the validation data
            arma::mat predictions;
            network.Predict(validationData, predictions);

            if ((epoch % predictionsInterval) == 0) {
                predictions.save(predictionsPath + "\\predictions" + std::to_string(epoch) + ".csv", arma::csv_ascii);
            }

            // TODO: ABS statt SQUARE
            // Calculate the mean squared error
            double loss = arma::accu(arma::square(predictions - validationLabels)) / predictions.n_cols;

            output << loss << std::endl;

            // Check for stop
            if (!earlyStopAtMinLoss) return false;

            if (loss < bestLoss) {
                bestLoss = loss;
                steps = 0;
                return false;
            }
            if (++steps >= patience) {
                predictions.save(predictionsPath + "\\predictions" + std::to_string(epoch) + ".csv", arma::csv_ascii);
                std::cout << "Validation loss did not decrease for " << patience
                    << " epochs. Terminating optimization.\n";
                return true;
            }
            return false;
        }
    };

} // namespace ens

#endif
