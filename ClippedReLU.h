#ifndef MLPACK_METHODS_ANN_ACTIVATION_FUNCTIONS_CLIPPED_RECTIFIER_FUNCTION_HPP
#define MLPACK_METHODS_ANN_ACTIVATION_FUNCTIONS_CLIPPED_RECTIFIER_FUNCTION_HPP

#include <mlpack/prereqs.hpp>
#include <algorithm>

namespace mlpack {
    namespace ann /** Artificial Neural Network. */ {

        /**
         * The rectifier function, defined by
         *
         * @f{eqnarray*}{
         * f(x) &=& \max(0, x) \\
         * f'(x) &=& \left\{
         *   \begin{array}{lr}
         *     1 & : x > 0 \\
         *     0 & : x \le 0
         *   \end{array}
         * \right.
         * @f}
         */
        class ClippedRectifierFunction
        {
        public:

            // Returns CReLU(x)
            static double Fn(const double x)
            {
                return std::max(0.0, std::min(x, 1.0));
            }

            // y = CReLU(x)
            // Matrix variant
            template<typename eT>
            static void Fn(const arma::Mat<eT>& x, arma::Mat<eT>& y)
            {
                y.zeros(x.n_rows, x.n_cols);
                arma::mat z;
                z.ones(x.n_rows, x.n_cols);
                y = arma::max(y, arma::min(x, z));
            }

            // y = CReLU(x)
            // Cube variant
            template<typename eT>
            static void Fn(const arma::Cube<eT>& x, arma::Cube<eT>& y)
            {
                y.zeros(x.n_rows, x.n_cols, x.n_slices);
                arma::cube z;
                z.ones(x.n_rows, x.n_cols, x.n_slices);
                y = arma::max(y, arma::min(x, z));
            }

            // Returns CReLU'(x)
            static double Deriv(const double x)
            {
                return (double)((x > 0.0) && (x < 1.0));
            }

            // x = CReLU'(y)
            template<typename InputType, typename OutputType>
            static void Deriv(const InputType& y, OutputType& x)
            {
                x.set_size(arma::size(y));

                for (size_t i = 0; i < y.n_elem; ++i)
                    x(i) = Deriv(y(i));
            }
        }; // class ClippedRectifierFunction

        template <class ActivationFunction = ClippedRectifierFunction,
            typename InputDataType = arma::mat,
            typename OutputDataType = arma::mat>
        using ClippedReLULayer = BaseLayer<ActivationFunction, InputDataType, OutputDataType>;

    } // namespace ann
} // namespace mlpack

#endif

