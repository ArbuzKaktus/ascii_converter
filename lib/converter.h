#pragma once

#include <cstddef>
#include <opencv2/core/mat.hpp>
#include <opencv2/videoio.hpp>
#include <ostream>
#include <string>
#include <vector>

class Converter {
public:
  Converter(const std::string& path, std::ostream& output, bool colored, bool video);
  void convert(size_t height, int more_brightness, float saturation);
  void animate(size_t height, int more_brightness, float saturation, double custom_fps = -1);

private:
  std::string frameToAscii(const cv::Mat& frame, size_t height, int more_brightness, float saturation);

  cv::Mat image_;
  cv::VideoCapture video_;
  std::vector<cv::Mat> gif_frames_;
  bool is_gif_ = false;
  std::ostream& output_;
  bool colored_;
};