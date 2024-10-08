#ifndef UNSCENTED_TRANSFORM_H
#define UNSCENTED_TRANSFORM_H

#include "Eigen/Dense"
#include "Eigen/src/Cholesky/LLT.h"
#include "types.hpp"
#include "util.hpp"
#include <iostream>

namespace kf {
template <int32_t DIM> class UnscentedTransform {
public:
  static constexpr int32_t SIGMA_DIM{(2 * DIM) + 1};

  UnscentedTransform() {}
  ~UnscentedTransform() {}

  float32_t weight0() const { return _weights[0]; }
  float32_t weighti() const { return _weights[1]; }

  ///
  /// @brief algorithm to execute weight and sigma points calculation
  /// @param vecX input mean vector
  /// @param matPxx input covariance matrix
  /// @param kappa design parameter
  ///
  void compute(const Vector<DIM> &vecX, const Matrix<DIM, DIM> &matPxx,
               const float32_t kappa = 0.0F) {
    // 1. calculate weights
    updateWeights(kappa);

    // 2. update sigma points _sigmaX
    updateSigmaPoints(vecX, matPxx, kappa);
  }

  template <int32_t DIM_X>
  void
  calculateWeightedMeanAndCovariance(const Matrix<DIM_X, SIGMA_DIM> &sigmaX,
                                     Vector<DIM_X> &vecX,
                                     Matrix<DIM_X, DIM_X> &matPxx) {
    // 1. calculate mean: \bar{y} = \sum_{i_0}^{2n} W[0, i] Y[:, i]
    vecX = _weights[0] * util::getColumnAt<DIM_X, SIGMA_DIM>(0, sigmaX);
    for (int32_t i{1}; i < SIGMA_DIM; ++i) {
      vecX += _weights[1] * util::getColumnAt<DIM_X, SIGMA_DIM>(
                                i, sigmaX); // y += W[0, i] Y[:, i]
    }

    // 2. calculate covariance: P_{yy} = \sum_{i_0}^{2n} W[0, i] (Y[:, i] -
    // \bar{y}) (Y[:, i] - \bar{y})^T
    Vector<DIM_X> devXi{util::getColumnAt<DIM_X, SIGMA_DIM>(0, sigmaX) -
                        vecX}; // Y[:, 0] - \bar{ y }
    matPxx = _weights[0] * devXi *
             devXi.transpose(); // P_0 = W[0, 0] (Y[:, 0] - \bar{y}) (Y[:, 0] -
                                // \bar{y})^T

    for (int32_t i{1}; i < SIGMA_DIM; ++i) {
      devXi = util::getColumnAt<DIM_X, SIGMA_DIM>(i, sigmaX) -
              vecX; // Y[:, i] - \bar{y}

      const Matrix<DIM_X, DIM_X> Pi{
          _weights[1] * devXi *
          devXi.transpose()}; // P_i = W[0, i] (Y[:, i] - \bar{y}) (Y[:, i] -
                              // \bar{y})^T

      matPxx += Pi; // y += W[0, i] (Y[:, i] - \bar{y}) (Y[:, i] - \bar{y})^T
    }
  }

  ///
  /// @brief algorithm to execute transforming sigma points through nonlinear
  /// function and calculate the updated weights and covariance.
  /// @param nonlinearFunction nonlinear function callback
  /// @param vecY output mean vector (weighted mean vector)
  /// @param matPyy output covariance matrix (weighted covariance matrix)
  ///
  template <typename NonLinearFunctionCallback>
  void transform(NonLinearFunctionCallback nonlinearFunction, Vector<DIM> &vecY,
                 Matrix<DIM, DIM> &matPyy) {
    Matrix<DIM, SIGMA_DIM> sigmaY;

    // 3. transform sigma points X through the nonlinear model f(X) to obtain Y
    // sigma points
    transformSigmaPoints(nonlinearFunction, sigmaY);

    // 4. calculate weight mean and covariance from the transformed sigma points
    // Y and weights
    updateTransformedMeanAndCovariance(sigmaY, vecY, matPyy);
  }

  void showSummary() {
    std::cout << "DIM_N : " << DIM << "\n";
    std::cout << "DIM_M : " << SIGMA_DIM << "\n";
    std::cout << "_weights[0] : \n" << _weights[0] << "\n";
    std::cout << "_weights[1] : \n" << _weights[1] << "\n";
    std::cout << "_sigmaX : \n" << _sigmaX << "\n";
    std::cout << "_sigmaY : \n" << _sigmaY << "\n";
  }

  Matrix<1, 2> &weights() { return _weights; }
  const Matrix<1, 2> &weights() const { return _weights; }

  Matrix<DIM, SIGMA_DIM> &sigmaX() { return _sigmaX; }
  const Matrix<DIM, SIGMA_DIM> &sigmaX() const { return _sigmaX; }

  Matrix<DIM, SIGMA_DIM> &sigmaY() { return _sigmaY; }
  const Matrix<DIM, SIGMA_DIM> &sigmaY() const { return _sigmaY; }

private:
  /// @brief unscented transform weight 0 for mean
  Matrix<1, 2> _weights;

  /// @brief input sigma points
  Matrix<DIM, SIGMA_DIM> _sigmaX;

  /// @brief output sigma points
  Matrix<DIM, SIGMA_DIM> _sigmaY;

  ///
  /// @brief algorithm to calculate the weights used to draw the sigma points
  /// @param kappa design scaling parameter for sigma points selection
  ///
  void updateWeights(float32_t kappa) {
    static_assert(DIM > 0, "DIM is Zero which leads to numerical issue.");

    const float32_t denoTerm{kappa + static_cast<float32_t>(DIM)};

    _weights[0] = kappa / denoTerm;
    _weights[1] = 0.5F / denoTerm;

    std::cout << "weight_0: " << _weights[0] << std::endl;
    std::cout << "weight_i: " << _weights[1] << std::endl;
  }

  ///
  /// @brief algorithm to calculate the deterministic sigma points for
  /// the unscented transformation
  ///
  /// @param vecX mean of the normally distributed state
  /// @param matPxx covariance of the normally distributed state
  /// @param kappa design scaling parameter for sigma points selection
  ///
  void updateSigmaPoints(const Vector<DIM> &vecX,
                         const Matrix<DIM, DIM> &matPxx,
                         const float32_t kappa) {
    std::cout << "n + kappa: " << DIM + kappa << std::endl;
    const float32_t scalarMultiplier{
        std::sqrt(DIM + kappa)}; // sqrt(n + \kappa)

    // cholesky factorization to get matrix Pxx square-root
    Eigen::LLT<Matrix<DIM, DIM>> lltOfPxx(matPxx);
    Matrix<DIM, DIM> matSxx{lltOfPxx.matrixL()}; // sqrt(P_{xx})
    std::cout << "square-root of cov:\n" << matSxx << std::endl;

    matSxx *= scalarMultiplier; // sqrt( (n + \kappa) * P_{xx} )
    std::cout << "sqrt( (n + kappa) * P_{xx} ):\n" << matSxx << std::endl;

    // X_0 = \bar{x}
    util::copyToColumn<DIM, SIGMA_DIM>(0, _sigmaX, vecX);

    for (int32_t i{0}; i < DIM; ++i) {
      const int32_t IDX_1{i + 1};
      const int32_t IDX_2{i + DIM + 1};

      util::copyToColumn<DIM, SIGMA_DIM>(IDX_1, _sigmaX, vecX);
      util::copyToColumn<DIM, SIGMA_DIM>(IDX_2, _sigmaX, vecX);

      const Vector<DIM> vecShiftTerm{util::getColumnAt<DIM, DIM>(i, matSxx)};
      std::cout << "id: " << i << ", origin _sigmaX:\n" << _sigmaX << std::endl;
      std::cout << "id: " << i << ", col of sqrt( (n + kappa) * P_{xx} ):\n"
                << vecShiftTerm << std::endl;

      util::addColumnFrom<DIM, SIGMA_DIM>(
          IDX_1, _sigmaX,
          vecShiftTerm); // X_i     = \bar{x} + sqrt( (n + \kappa) * P_{xx} )
      std::cout << "id: " << i << ", add _sigmaX:\n" << _sigmaX << std::endl;

      util::subColumnFrom<DIM, SIGMA_DIM>(
          IDX_2, _sigmaX,
          vecShiftTerm); // X_{i+n} = \bar{x} - sqrt( (n + \kappa) * P_{xx} )
      std::cout << "id: " << i << ", sub _sigmaX:\n" << _sigmaX << std::endl;
    }
  }

  ///
  /// @brief transform sigma points X through the nonlinear function
  /// @param nonlinearFunction nonlinear function to be used
  /// @param sigmaY output transformed sigma points from the nonlinear function
  ///
  template <typename NonLinearFunctionCallback>
  void transformSigmaPoints(NonLinearFunctionCallback nonlinearFunction,
                            Matrix<DIM, SIGMA_DIM> &sigmaY) {
    for (int32_t i{0}; i < SIGMA_DIM; ++i) {
      const Vector<DIM> x{util::getColumnAt<DIM, SIGMA_DIM>(i, _sigmaX)};
      const Vector<DIM> y{nonlinearFunction(x)}; // y = f(x)

      util::copyToColumn<DIM, SIGMA_DIM>(i, sigmaY, y); // Y[:, i] = y
      std::cout << "sigma Y: \n" << sigmaY << std::endl;
    }
    _sigmaY = sigmaY;
  }

  ///
  /// @brief calculate weighted mean and covariance given a set of sigma points
  /// with associated weights.
  /// @param sigmaY input transformed sigma points from the nonlinear function
  /// @param vecY output weighted mean vector
  /// @param matPyy output weighted covariance matrix
  ///
  void updateTransformedMeanAndCovariance(const Matrix<DIM, SIGMA_DIM> &sigmaY,
                                          Vector<DIM> &vecY,
                                          Matrix<DIM, DIM> &matPyy) {
    // 1. calculate mean: \bar{y} = \sum_{i_0}^{2n} W[0, i] Y[:, i]
    vecY = _weights[0] * util::getColumnAt<DIM, SIGMA_DIM>(0, sigmaY);
    for (int32_t i{1}; i < SIGMA_DIM; ++i) {
      vecY += _weights[1] * util::getColumnAt<DIM, SIGMA_DIM>(
                                i, sigmaY); // y += W[0, i] Y[:, i]
    }
    std::cout << "Y mean: \n" << vecY << std::endl;

    // 2. calculate covariance: P_{yy} = \sum_{i_0}^{2n} W[0, i] (Y[:, i] -
    // \bar{y}) (Y[:, i] - \bar{y})^T
    Vector<DIM> devYi{util::getColumnAt<DIM, SIGMA_DIM>(0, sigmaY) -
                      vecY}; // Y[:, 0] - \bar{ y }
    matPyy = _weights[0] * devYi *
             devYi.transpose(); // P_0 = W[0, 0] (Y[:, 0] - \bar{y}) (Y[:, 0] -
                                // \bar{y})^T

    for (int32_t i{1}; i < SIGMA_DIM; ++i) {
      devYi = util::getColumnAt<DIM, SIGMA_DIM>(i, sigmaY) -
              vecY; // Y[:, i] - \bar{y}

      const Matrix<DIM, DIM> Pi{
          _weights[1] * devYi *
          devYi.transpose()}; // P_i = W[0, i] (Y[:, i] - \bar{y}) (Y[:, i] -
                              // \bar{y})^T

      matPyy += Pi; // y += W[0, i] (Y[:, i] - \bar{y}) (Y[:, i] - \bar{y})^T
    }
    std::cout << "Y covariance: \n" << matPyy << std::endl;
  }
};
} // namespace kf

#endif // UNSCENTED_TRANSFORM_H
