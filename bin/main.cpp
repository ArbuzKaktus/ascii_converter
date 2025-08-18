#include "input_manager.h"
#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char** argv) {
  cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_ERROR);
  
  InputManager::StartProgramm(std::cout, argc, argv);
  return 0;
}