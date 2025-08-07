#include "converter.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>

namespace {
  const std::string kSymbols = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
  const std::string kReverseSymbols = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
}


std::string get_ansi_color(int r, int g, int b) {
    return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
}

Converter::Converter(const std::string& path ,std::ostream& output, bool colored):
  output_(output), colored_(colored) {
    if (colored) {
      image_ = cv::imread(path);
    }
    else {
      image_ = cv::imread(path, cv::IMREAD_GRAYSCALE);
    }

    if (image_.empty()) {
      std::cerr << "Error while opening image";
      throw std::runtime_error("error while reading file");
      exit(EXIT_FAILURE);
    }
    
}

void Converter::convert(size_t height, bool reverse) {
  std::string symbols = reverse ? kReverseSymbols : kSymbols;
  cv::Mat resized_image;
  size_t width = static_cast<int>(image_.cols * (height / static_cast<double>(image_.rows)) * 2);
  cv::resize(image_, resized_image ,cv::Size(width, height));

  if (colored_) {
    for (size_t y = 0; y < static_cast<size_t>(resized_image.rows); y++) {
        for (size_t x = 0; x < static_cast<size_t>(resized_image.cols); x++) {
            cv::Vec3b pixel = resized_image.at<cv::Vec3b>(y, x);
            int r = pixel[2];
            int g = pixel[1];
            int b = pixel[0];

            int brightness = static_cast<int>(0.299 * r + 0.587 * g + 0.114 * b);
            size_t index = static_cast<size_t>((brightness / 255.0) * (symbols.size() - 1));

            output_ << get_ansi_color(r, g, b) + symbols[index];
        }
        output_ << "\033[0m\n";
    }
  }
  else {

    for (size_t y = 0; y < resized_image.rows; ++y) {
      for (size_t x = 0; x < resized_image.cols; ++x) {
        uint8_t brightness = resized_image.at<uint8_t>(y, x);
  
        size_t index = static_cast<size_t>(brightness / 255.0 * (symbols.size() - 1));
        output_ << symbols[index];
      }
      output_ << std::endl;
    }
  }
}
