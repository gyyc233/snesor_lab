#include "discrete_kalman_filter.h"
#include "matplotlibcpp.h"
#include <Eigen/Dense>
#include <iostream>
#include <memory>
#include <vector>

namespace plt = matplotlibcpp;

// Test for the KalmanFilter class with 1D projectile motion. 1维平抛运动
// 状态量有３个 位置 速度 加速度,这三者的时间状态方程如下
// x_(k+1)=x_k+v_k*t+1/2*a_k*t^2
// v_(k+1)=v_k+a_k*t
// a_(k+1)=a_k;

// 观测方程
// y_(k+1)=H*x_(k+1);
int main() {
  int n = 3;            // Number of states
  int m = 1;            // Number of measurements
  double dt = 1.0 / 30; // Time step

  Eigen::MatrixXd A(n, n); // System dynamics matrix
  Eigen::MatrixXd H(m, n); // Output matrix
  Eigen::MatrixXd Q(n, n); // Process noise covariance
  Eigen::MatrixXd R(m, m); // Measurement noise covariance
  Eigen::MatrixXd P(n, n); // Estimate error covariance

  // Discrete LTI projectile motion, measuring position only
  A << 1, dt, 0.5 * dt * dt, 0, 1, dt, 0, 0, 1;
  H << 1, 0, 0;

  Q << .05, .05, .0, .05, .05, .0, .0, .0, .0;
  R << 5;
  P << .1, .1, .1, .1, 10000, 10, .1, 10, 100;

  std::cout << "A: \n" << A << std::endl;
  std::cout << "H: \n" << H << std::endl;
  std::cout << "Q: \n" << Q << std::endl;
  std::cout << "R: \n" << R << std::endl;
  std::cout << "P: \n" << P << std::endl;

  DiscreteKalmanFilter dkf(dt, A, H, Q, R, P);

  // List of noisy position measurements (y)
  // 大概是一个炮弹发射的过程
  std::vector<double> measurements = {
      1.04202710058,  1.10726790452,  1.2913511148,    1.48485250951,
      1.72825901034,  1.74216489744,  2.11672039768,   2.14529225112,
      2.16029641405,  2.21269371128,  2.57709350237,   2.6682215744,
      2.51641839428,  2.76034056782,  2.88131780617,   2.88373786518,
      2.9448468727,   2.82866600131,  3.0006601946,    3.12920591669,
      2.858361783,    2.83808170354,  2.68975330958,   2.66533185589,
      2.81613499531,  2.81003612051,  2.88321849354,   2.69789264832,
      2.4342229249,   2.23464791825,  2.30278776224,   2.02069770395,
      1.94393985809,  1.82498398739,  1.52526230354,   1.86967808173,
      1.18073207847,  1.10729605087,  0.916168349913,  0.678547664519,
      0.562381751596, 0.355468474885, -0.155607486619, -0.287198661013,
      -0.602973173813};

  // Best guess of initial states
  Eigen::VectorXd x0(n);
  double t = 0;
  x0 << measurements[0], 0, -9.81; // 这里例子给的速度初值感觉不对
  dkf.init(t, x0);

  // Feed measurements into filter, output estimated states

  Eigen::VectorXd y(m);
  std::cout << "t = " << t << ", "
            << "x_hat[0]: " << dkf.state().transpose() << std::endl;

  std::vector<double> test_index;
  std::vector<double> test_y;
  std::vector<double> test_correct;

  for (int i = 0; i < measurements.size(); i++) {
    std::cout << "============ index: " << i << "============" << std::endl;
    t += dt;
    y << measurements[i];
    dkf.update(y);
    std::cout << "t = " << t << ", "
              << "y[" << i << "] = " << y.transpose() << ", x_hat[" << i
              << "] = " << dkf.state().transpose() << std::endl;

    test_index.push_back(i);
    test_y.push_back(y.transpose()(0, 0));
    test_correct.push_back(dkf.state().transpose()(0, 0));
  }

  plt::plot(test_index, test_y);
  plt::plot(test_index, test_correct);
  plt::show();
  return 0;
}
