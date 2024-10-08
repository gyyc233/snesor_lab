#include <iostream>
#include <stdint.h>

#include "kalman_filter.hpp"
#include "types.hpp"

// state dimensions
static constexpr size_t DIM_X{2};
// measure dimensions
static constexpr size_t DIM_Z{1};
// time step
static constexpr kf::float32_t T{1.0F};
static constexpr kf::float32_t Q11{0.1F}, Q22{0.1F};

static kf::KalmanFilter<DIM_X, DIM_Z> kalman_filter;

void executePredictionStep();
void executeCorrectionStep();

int main() {
  executePredictionStep();
  executeCorrectionStep();

  return 0;
}

void executePredictionStep() {
  kalman_filter.vecX() << 0.0F, 2.0F;
  kalman_filter.matP() << 0.1F, 0.0F, 0.0F, 0.1F;

  kf::Matrix<DIM_X, DIM_X> F; // state transition matrix 状态转移矩阵
  F << 1.0F, T, 0.0F, 1.0F;

  kf::Matrix<DIM_X, DIM_X> Q; // process noise covariance
  Q(0, 0) = (Q11 * T) + (Q22 * (std::pow(T, 3.0F) / 3.0F));
  Q(0, 1) = Q(1, 0) = Q22 * (std::pow(T, 2.0F) / 2.0F);
  Q(1, 1) = Q22 * T;

  kalman_filter.predictLKF(F, Q); // execute prediction step

  std::cout << "\npredicted state vector = \n" << kalman_filter.vecX() << "\n";
  std::cout << "\npredicted state covariance = \n"
            << kalman_filter.matP() << "\n";
}

void executeCorrectionStep() {
  kf::Vector<DIM_Z> vecZ;
  vecZ << 2.25F;

  kf::Matrix<DIM_Z, DIM_Z> matR;
  matR << 0.01F;

  kf::Matrix<DIM_Z, DIM_X> matH;
  matH << 1.0F, 0.0F;

  kalman_filter.correctLKF(vecZ, matR, matH);

  std::cout << "\ncorrected state vector = \n" << kalman_filter.vecX() << "\n";
  std::cout << "\ncorrected state covariance = \n"
            << kalman_filter.matP() << "\n";
}
