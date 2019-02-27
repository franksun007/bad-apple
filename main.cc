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
DEFINE_int32(subsample, 2, "Subsamples of each frame");
DEFINE_int32(fps, 30, "The FPS to play on the terminal");

namespace {

// TODO(Frank): support multiple resolution. Read Image size directly from ppm.
constexpr int32_t kColorComponent = 3;
constexpr int32_t kImageWidth = 480;
constexpr int32_t kImageHeight = 360;
constexpr int32_t kRgbSize = kImageWidth * kImageHeight * kColorComponent;

std::vector<uint8_t> readPPM(const std::string &filename) {
  constexpr int32_t kPPMHeaderSize = 15;
  std::ifstream file(filename.c_str(), std::ios::binary);

  std::vector<uint8_t> data;
  data.resize(kRgbSize);

  file.ignore(kPPMHeaderSize);
  file.read((char *)data.data(), kRgbSize);

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
  int result = 0;
  setupterm(NULL, STDOUT_FILENO, &result);
  for (int j = 0; j < kImageHeight; j += FLAGS_subsample) {
    for (int i = 0; i < kStride - (kColorComponent * FLAGS_subsample - 1);
         i += kColorComponent * FLAGS_subsample) {
      int sum = 0;
      for (int k = 0; k < kColorComponent * FLAGS_subsample; k++) {
        sum += data[j * kStride + i + k];
      }
      sum /= FLAGS_subsample;

      // TODO(Frank): Use switch?
      // TODO(Frank): None linear
      if (sum < 20) {
        ss << "\033[1;31m0";
      } else if (sum < 500) {
        int randnum = ((unsigned int)(j + i + sum + std::rand())) % 4 + 2;
        ss << "\033[1;3" << randnum << "m" << randnum;
      } else {
        ss << "\033[1;36m1";
      }
    }
    ss << "\n";
  }
  return ss.str();
}

void PrintFrame(const std::string &string) {
  printf("%s\n", string.c_str());
  // TODO(Frank): magic number
  usleep((int32_t)(0.03333333333 * 1000000 * 30 / FLAGS_fps));
}

} // namespace

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  const std::vector<std::string> all_filelist = GetListOfFiles(FLAGS_frames);
  std::vector<std::string> all_frame;
  all_frame.resize(all_filelist.size());
  // Served as a threadpool.
  constexpr int32_t kChunkSize = 32;
  std::future<std::string> pointer[kChunkSize];

  size_t fl_index = 0;
  for (; fl_index < all_filelist.size() - (kChunkSize - 1);
       fl_index += kChunkSize) {
    // Schedule the work
    for (int i = 0; i < kChunkSize; i++) {
      pointer[i] = std::async(std::launch::async, ProcessFrame,
                              readPPM(all_filelist[fl_index + i]));
    }
    // Sync
    for (int i = 0; i < kChunkSize; i++) {
      all_frame[fl_index + i] = pointer[i].get();
    }
  }

  // Tail work
  for (; fl_index < all_filelist.size(); fl_index++) {
    all_frame[fl_index] =
        ProcessFrame(readPPM(all_filelist[fl_index]));
  }

  // Print Frame
  // TODO(Frank): 30 - magic number
  for (size_t i = 0; i < all_frame.size(); i += 30 / FLAGS_fps) {
    PrintFrame(all_frame[i]);
  }
}
