#pragma once

#include <cstddef>
#include <fstream>
#include <opencv2/core/mat.hpp>
#include <ostream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <string>

class Converter {
public:
  Converter(const std::string& path ,std::ostream& output);
  void convert(size_t height, bool reverse);

private:
  cv::Mat image_;
  std::ostream& output_;
};