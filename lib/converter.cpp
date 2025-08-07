#include "converter.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>

namespace {
  const std::string symbols = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
  const std::string reverse_symbols = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
}

Converter::Converter(const std::string& path ,std::ostream& output):
  output_(output) {
    image_ = cv::imread(path, cv::IMREAD_GRAYSCALE);

    if (image_.empty()) {
      std::cerr << "Error while opening image";
      throw std::runtime_error("error while reading file");
    }
    
}

void Converter::convert(size_t height, bool reverse) {
  cv::Mat resized_image;
  size_t width = static_cast<int>(image_.cols * (height / static_cast<double>(image_.rows)) * 2);
  cv::resize(image_, resized_image ,cv::Size(width, height));

  for (size_t y = 0; y < resized_image.rows; ++y) {
    for (size_t x = 0; x < resized_image.cols; ++x) {
      uint8_t brightness = resized_image.at<uint8_t>(y, x);

      size_t index = static_cast<size_t>(brightness / 255.0 * (symbols.size() - 1));
      if (reverse) {
        output_ << reverse_symbols[index];
      }
      else {
        output_ << symbols[index];
      }
    }
    output_ << std::endl;
  }
  // cv::imshow("window", resized_image);
  // cv::waitKey(0);
}
