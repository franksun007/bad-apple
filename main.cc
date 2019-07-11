#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <term.h>
#include <time.h>
#include <unistd.h>
#include <vector>

#include "gflags/gflags.h"

DEFINE_string(frames, "", "The folder contains all the frames");
// TODO(Frank): support fractional subsampling
DEFINE_int32(subsample, 2, "Subsamples of each frame");
DEFINE_int32(fps, 30, "The FPS to play on the terminal");
DEFINE_int32(acceleration, 1,
             "The acceleration x you want to apply. More for debug");

namespace {

// TODO(Frank): support multiple resolution. Read Image size directly from ppm.
constexpr int32_t kColorComponent = 3;
constexpr int32_t kImageWidth = 480;
constexpr int32_t kImageHeight = 360;
constexpr int32_t kRgbSize = kImageWidth * kImageHeight * kColorComponent;

std::vector<uint8_t> ReadPPM(const std::string &filename) {
  constexpr int32_t kPPMHeaderSize = 15;
  std::ifstream file(filename.c_str(), std::ios::binary);

  std::vector<uint8_t> data;
  data.resize(kRgbSize);

  file.ignore(kPPMHeaderSize);
  // TODO(Frank): Unsafe
  file.read((char *)data.data(), kRgbSize);
  file.close();

  return data;
}

const std::vector<std::string> GetListOfFiles(const std::string &base_dir) {
  std::vector<std::string> result;
  for (const auto &entry : std::filesystem::directory_iterator(base_dir)) {
    result.push_back(entry.path());
  }
  std::sort(result.begin(), result.end());
  return result;
}

std::string ProcessFrame(const std::vector<uint8_t> data) {
  std::stringstream ss;
  constexpr int32_t kStride = kImageWidth * kColorComponent;
  for (int j = 0; j < kImageHeight; j += FLAGS_subsample) {
    int32_t prev_result = -1;
    for (int i = 0; i < kStride - (kColorComponent * FLAGS_subsample - 1);
         i += kColorComponent * FLAGS_subsample) {
      int sum = 0;
      for (int k = 0; k < kColorComponent * FLAGS_subsample; k++) {
        sum += data[j * kStride + i + k];
      }
      sum /= FLAGS_subsample;

      // TODO(Frank): move to namespace {}
      constexpr char kBlack[] = "\033[1;30m";
      constexpr char kRed[] = "\033[1;31m";
      constexpr char kWhite[] = "\033[1;97m";
      constexpr char kCyan[] = "\033[1;36m";
      constexpr char kLightGray[] = "\033[1;37m";
      constexpr char kDarkGray[] = "\033[1;90m";

      // TODO(Frank): Use make switch?
      // TODO(Frank): None linear
      if (sum < 20) {

        if (prev_result == 0) {
          ss << "0";
        } else {
          ss << kBlack << "0";
        }
        prev_result = 0;

      } else if (sum < 500) {
        int randnum = (j + i + sum + std::rand()) % 4 + 2;
        if (randnum % 4 == 0) {
          ss << kDarkGray << randnum;
        } else if (randnum % 4 == 1) {
          ss << kWhite << randnum;
        } else if (randnum % 4 == 2) {
          ss << kBlack << randnum;
        } else {
          ss << kLightGray << randnum;
        }

        /*
        if (prev_result == 2) {
            ss << randnum;
        } else {
            ss << "\033[1;3" << randnum << "m" << randnum;
        }
        */
        prev_result = 2;

      } else {

        if (prev_result == 1) {
          ss << "1";
        } else {
          ss << kWhite << "1";
        }
        prev_result = 1;
      }
      ss << " ";
    }
    ss << "\n";
  }
  return ss.str();
}

void PrintFrame(const std::string &string) {
  int result = 0;
  setupterm(NULL, STDOUT_FILENO, &result);
  putp(tigetstr("clear"));
  std::cout << string << std::endl;
  // TODO(Frank): magic number
  usleep(
      (int32_t)(0.03333333333 * 1000000 * 30 / FLAGS_fps / FLAGS_acceleration));
}

} // namespace

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  const std::vector<std::string> all_filelist = GetListOfFiles(FLAGS_frames);
  std::vector<std::string> all_frame(all_filelist.size());
  // TODO(Frank): Too much memory. Implement a threadpool.
  std::vector<std::future<std::string>> handlers(all_filelist.size());

  // Schedule the work
  for (size_t i = 0; i < all_filelist.size(); ++i) {
    handlers[i] =
        std::async(std::launch::async, ProcessFrame, ReadPPM(all_filelist[i]));
  }

  // Sync
  for (size_t i = 0; i < all_filelist.size(); ++i) {
    all_frame[i] = handlers[i].get();
  }

  // Print Frame
  // TODO(Frank): 30 - magic number
  for (size_t i = 0; i < all_frame.size(); i += 30 / FLAGS_fps) {
    PrintFrame(all_frame[i]);
  }
}
