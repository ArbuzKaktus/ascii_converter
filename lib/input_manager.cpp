#include "input_manager.h"

#include <cstddef>
#include <iostream>
#include <opencv2/core/types.hpp>
#include <ostream>
#include <string>

#include "converter.h"

void InputManager::StartProgramm(std::ostream& output) {
  std::string image_path;
  size_t height;
  bool reverse;
  std::cout << "Enter Image Path:   ";
  std::cin >> image_path;
  Converter converter(image_path, output);
  std::cout << std::endl << "Enter height: ";
  std::cin >> height;
  std::cout << std::endl << "Need reverse? (0/1)";
  std::cin >> reverse;
  converter.convert(height, reverse);
}