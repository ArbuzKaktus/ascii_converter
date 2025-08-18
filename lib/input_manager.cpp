#include "input_manager.h"

#include <cstddef>
#include <iostream>
#include <string>

#include "converter.h"

void InputManager::StartProgramm(std::ostream& output, int argc, char** argv) {
  try {
    bool is_video_mode = false;

    if (argc >= 2 && std::string(argv[1]) == "-v") {
      is_video_mode = true;
    }
    
    std::string file_path;
    size_t height;
    char confirmation;
    bool reverse;
    bool colored;
    float saturation;
    double fps = -1;
    int more_brightness = 0;
    
    std::cout << "Enter File Path: ";
    std::cin >> file_path;
    std::cout << "Is Colored? (0/1): ";
    std::cin >> colored;
    
    Converter converter(file_path, output, colored, is_video_mode);
    
    std::cout << "Enter height: ";
    std::cin >> height;
    std::cout << "Need reverse? (0/1): ";
    std::cin >> reverse;
    std::cout << "How many brightness add: ";
    std::cin >> more_brightness;
    std::cout << "Saturation koef: ";
    std::cin >> saturation;
    
    if (is_video_mode) {
      std::cout << "Target FPS (0 for original speed): ";
      std::cin >> fps;
      if (fps == 0) fps = -1;
      std::cout << "PLAY? (any symbol): ";
      std::cin >> confirmation;
      converter.animate(height, reverse, more_brightness, saturation, fps);
    } else {
      converter.convert(height, reverse, more_brightness, saturation);
    }
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }
}