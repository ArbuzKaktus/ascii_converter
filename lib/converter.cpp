#include "converter.h"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <stdexcept>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#endif

namespace {
constexpr char kSymbols[] =
    {' ', '.', '\'', '`', '^', '\"', ',', ':', ';', 'I', 'l', '!',
      'i', '>', '<', '~', '+', '_' , '-', '?', ']', '[', '}', '{', '1',
    ')', '(', '|', '\\', '/', 't', 'f', 'j', 'r', 'x', 'n', 'u', 'v', 
    'c', 'z', 'X', 'Y', 'U', 'J', 'C', 'L', 'Q', '0', 'O', 'Z', 'm', 'w',
    'q', 'p', 'd', 'b', 'k', 'h', 'a', 'o', '*', '#', 'M', 'W', '&',
    '8', '%', 'B', '@', '$'};

constexpr double kDefaultFallbackFps = 25.0;
constexpr double kAspectRatioCorrection = 2.0;
constexpr int kBrightnessWeightRed = 299;
constexpr int kBrightnessWeightGreen = 587;
constexpr int kBrightnessWeightBlue = 114;
constexpr int kBrightnessWeightTotal = 1000;
constexpr int kMaxBrightness = 255;

void clearScreen() {
#ifdef _WIN32
  system("cls");
#else
  std::cout << "\033[2J\033[H" << std::flush;
#endif
}

void clearAndPosition() {
#ifdef _WIN32
  COORD coord = {0, 0};
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleCursorPosition(hConsole, coord);
#else
  std::cout << "\033[H" << std::flush;
#endif
}

void hideConsoleCursor() {
#ifdef _WIN32
  CONSOLE_CURSOR_INFO cursorInfo;
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = false;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
  std::cout << "\033[?25l" << std::flush;
#endif
}

void showConsoleCursor() {
#ifdef _WIN32
  CONSOLE_CURSOR_INFO cursorInfo;
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  GetConsoleCursorInfo(hConsole, &cursorInfo);
  cursorInfo.bVisible = true;
  SetConsoleCursorInfo(hConsole, &cursorInfo);
#else
  std::cout << "\033[?25h" << std::flush;
#endif
}

bool checkKeypress() {
#ifdef _WIN32
  return _kbhit() != 0;
#else
  return false;
#endif
}
} 

std::string get_ansi_color(int r, int g, int b) {
  return "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" +
         std::to_string(b) + "m";
}

cv::Mat increaseSaturation(cv::Mat image, float factor) {
  cv::Mat hsv;
  cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);

  std::vector<cv::Mat> channels;
  cv::split(hsv, channels);

  channels[1] = channels[1] * factor;
  cv::threshold(channels[1], channels[1], 255, 255, cv::THRESH_TRUNC);

  cv::merge(channels, hsv);
  cv::cvtColor(hsv, image, cv::COLOR_HSV2BGR);

  return image;
}

std::string Converter::frameToAscii(const cv::Mat& frame, size_t height,
                                    int more_brightness, float saturation) {

  if (frame.empty()) {
    std::cerr << "Warning: Empty frame encountered in frameToAscii" << std::endl;
    return "";
  }

  size_t width = static_cast<size_t>(
      frame.cols * (height / static_cast<double>(frame.rows)) * kAspectRatioCorrection);
  
  cv::Mat processed_frame;
  if (!colored_) {
    cv::cvtColor(frame, processed_frame, cv::COLOR_BGR2GRAY);
  } else {
    processed_frame = frame.clone();
  }

  cv::Mat resized_image;
  cv::resize(processed_frame, resized_image, cv::Size(width, height));

  std::string asciiArt;
  asciiArt.reserve(width * height * (colored_ ? 20 : 1));
  
  if (colored_) {
    cv::Mat brightened;
    resized_image.convertTo(brightened, -1, 1.3, more_brightness);
    brightened = increaseSaturation(brightened, saturation);
    
    const cv::Vec3b* row_ptr;
    for (int y = 0; y < resized_image.rows; y++) {
      row_ptr = brightened.ptr<cv::Vec3b>(y);
      for (int x = 0; x < resized_image.cols; x++) {
        const cv::Vec3b& pixel = row_ptr[x];
        int r = pixel[2];
        int g = pixel[1];
        int b = pixel[0];

        int brightness = (kBrightnessWeightRed * r + kBrightnessWeightGreen * g + kBrightnessWeightBlue * b) / kBrightnessWeightTotal;
        size_t index = (brightness * (std::size(kSymbols) - 1)) / kMaxBrightness;

        asciiArt += get_ansi_color(r, g, b) + kSymbols[index];
      }
      asciiArt += "\033[0m\n";
    }
  } else {
    const uint8_t* row_ptr;
    for (int y = 0; y < resized_image.rows; y++) {
      row_ptr = resized_image.ptr<uint8_t>(y);
      for (int x = 0; x < resized_image.cols; x++) {
        uint8_t brightness = row_ptr[x];
        size_t index = (brightness * (std::size(kSymbols) - 1)) / kMaxBrightness;
        asciiArt += kSymbols[index];
      }
      asciiArt += '\n';
    }
  }
  return asciiArt;
}

void Converter::animate(size_t height, int more_brightness,
                        float saturation, double custom_fps) {

  double original_fps = video_.get(cv::CAP_PROP_FPS);
  if (original_fps <= 0 || std::isnan(original_fps)) {
    original_fps = kDefaultFallbackFps;
  }
  
  double target_fps;
  if (custom_fps > 0) {
    target_fps = custom_fps;
  } else {
    target_fps = original_fps;
  }
  
  int frameDelay = static_cast<int>(1000.0 / target_fps);
  
  double frame_skip_ratio = original_fps / target_fps;
  int frame_counter = 0;

  if (is_gif_ && !gif_frames_.empty()) {
    std::cout << "Press any key to stop animation..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    hideConsoleCursor();
    clearScreen();
    
    while (true) {
      for (size_t i = 0; i < gif_frames_.size(); i++) {
        if (gif_frames_[i].empty()) {
          std::cerr << "Warning: Empty frame in GIF, skipping" << std::endl;
          continue;
        }
        
        frame_counter++;
        if (frame_skip_ratio > 1.0 && frame_counter % static_cast<int>(frame_skip_ratio) != 0) {
          continue;
        }
        
        if (checkKeypress()) {
          showConsoleCursor();
          std::cout << "\nAnimation stopped by user." << std::endl;
          return;
        }
        
        auto start = std::chrono::steady_clock::now();
        std::string asciiFrame = frameToAscii(gif_frames_[i].clone(), height, more_brightness, saturation);
        
        clearAndPosition();
        std::cout << asciiFrame << std::flush;
        
        auto end = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        if (elapsed < frameDelay) {
          std::this_thread::sleep_for(std::chrono::milliseconds(frameDelay - elapsed));
        }
      }
    }
    showConsoleCursor();
    return;
  }

  cv::Mat frame;
  bool first_frame = true;
  frame_counter = 0;
  
  while (video_.read(frame)) {
    if (frame.empty()) {
      std::cout << "\nEnd of video reached." << std::endl;
      break;
    }
    
    frame_counter++;
    if (frame_skip_ratio > 1.0 && frame_counter % static_cast<int>(frame_skip_ratio) != 0) {
      continue;
    }
    
    if (first_frame) {
      std::cout << "Press any key to stop video..." << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      hideConsoleCursor();
      clearScreen();
      first_frame = false;
    }
    
    if (checkKeypress()) {
      showConsoleCursor();
      std::cout << "\nVideo stopped by user." << std::endl;
      return;
    }
    
    auto start = std::chrono::steady_clock::now();
    std::string asciiFrame = frameToAscii(frame, height, more_brightness, saturation);
    
    clearAndPosition();
    std::cout << asciiFrame << std::flush;

    auto end = std::chrono::steady_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    if (elapsed < frameDelay) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(frameDelay - elapsed));
    }
  }
  
  showConsoleCursor();
  std::cout << "\nVideo playback finished." << std::endl;
}

Converter::Converter(const std::string &path, std::ostream &output,
                     bool colored, bool video)
    : output_(output), colored_(colored) {

  auto toLower = [](std::string s){ for(char &c: s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return s; };
  std::string lowered = toLower(path);
  bool has_gif_ext = lowered.rfind(".gif") != std::string::npos;

  if (has_gif_ext) {
    std::vector<cv::Mat> frames;
    if (cv::imreadmulti(path, frames, cv::IMREAD_ANYCOLOR)) {
      if (!frames.empty() && !frames[0].empty()) {
        is_gif_ = true;
        gif_frames_ = std::move(frames);
        image_ = gif_frames_[0];
      }
    }
    if (!is_gif_) {
      std::cerr << "Failed to read GIF frames: " << path << "\n";
      throw std::runtime_error("Cannot read GIF");
    }
    return;
  }

  if (!video) {
    if (colored) {
      image_ = cv::imread(path, cv::IMREAD_COLOR);
    } else {
      image_ = cv::imread(path, cv::IMREAD_GRAYSCALE);
    }

    if (image_.empty()) {
      std::cerr << "Error while opening file: " << path << "\n";
      throw std::runtime_error("error while reading file");
    }
  } else {
    video_ = cv::VideoCapture{path};
    if (!video_.isOpened()) {
      std::cerr << "Error opening video file (unsupported format or missing codecs): " << path << "\n";
      throw std::runtime_error("error opening video");
    }
  }
}

void Converter::convert(size_t height, int more_brightness,
                        float saturation) {
  
  if (image_.empty()) {
    std::cerr << "Error: Image is empty, cannot convert" << std::endl;
    return;
  }
  
  cv::Mat resized_image;
  size_t width = static_cast<int>(
      image_.cols * (height / static_cast<double>(image_.rows)) * 2);
  cv::resize(image_, resized_image, cv::Size(width, height));
  if (colored_) {
    cv::Mat brightened;
    resized_image.convertTo(brightened, -1, 1.3, more_brightness);
    brightened = increaseSaturation(brightened, saturation);
    
    const cv::Vec3b* row_ptr;
    for (int y = 0; y < resized_image.rows; y++) {
      row_ptr = brightened.ptr<cv::Vec3b>(y);
      for (int x = 0; x < resized_image.cols; x++) {
        const cv::Vec3b& pixel = row_ptr[x];
        int r = pixel[2];
        int g = pixel[1];
        int b = pixel[0];

        int brightness = (299 * r + 587 * g + 114 * b) / 1000;
        size_t index = (brightness * (std::size(kSymbols) - 1)) / 255;

        output_ << get_ansi_color(r, g, b) + kSymbols[index];
      }
      output_ << "\033[0m\n";
    }
  } else {
    const uint8_t* row_ptr;
    for (int y = 0; y < resized_image.rows; y++) {
      row_ptr = resized_image.ptr<uint8_t>(y);
      for (int x = 0; x < resized_image.cols; x++) {
        uint8_t brightness = row_ptr[x];
        size_t index = (brightness * (std::size(kSymbols) - 1)) / 255;
        output_ << kSymbols[index];
      }
      output_ << '\n';
    }
  }
}
