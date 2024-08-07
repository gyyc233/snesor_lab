#ifndef __SENSORLAB_ORB_CV_H__
#define __SENSORLAB_ORB_CV_H__

#include "feature_detect.h"
#include "my_time.h"
#include <Eigen/Dense>
#include <Eigen/QR>
#include <Eigen/SVD>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace SensorLab {
class ORB_CV : public FeatureDetect {
public:
  ORB_CV();

  ~ORB_CV();

  void inputParams(const char *left_img, const char *right_img);

  bool output(std::vector<cv::DMatch> &match_result,
              std::vector<cv::KeyPoint> &key_points_img_l,
              std::vector<cv::KeyPoint> &key_points_img_r);

  void initialization() override;

  void run() override;

  void getData(std::vector<cv::Mat> &data);

private:
  cv::Mat img_l_;
  cv::Mat img_r_;

  cv::Ptr<cv::FeatureDetector> detector_;
  cv::Ptr<cv::DescriptorExtractor> orb_descriptor_;
  cv::Ptr<cv::DescriptorMatcher> matcher_;
  std::vector<cv::KeyPoint> key_points_img_l_;
  std::vector<cv::KeyPoint> key_points_img_r_;
  std::vector<cv::DMatch> good_matches_;
};
} // namespace SensorLab

#endif
