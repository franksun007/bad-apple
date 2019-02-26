#include <array>
#include <iostream>
#include <stdint.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <term.h>


#include <experimental/filesystem>

namespace {

constexpr int32_t kColorComponent = 3;
constexpr int32_t kImageWidth = 480;
constexpr int32_t kImageHeight = 360;
constexpr int32_t kRgbSize = kImageWidth * kImageHeight * kColorComponent;

std::vector<uint8_t> readPPM(const std::string& filename) {
    constexpr int32_t kPPMHeaderSize = 15;
    std::ifstream file(filename.c_str(), std::ios::binary);

    file.seekg (0, file.end);
    int length = file.tellg();
    file.seekg (0, file.beg);

    std::vector<uint8_t> data;
    data.resize(kRgbSize);

    file.ignore(kPPMHeaderSize);
    file.read((char *) data.data(), kRgbSize);
    
    return data;
}

const std::vector<std::string> GetListOfFiles(const std::string& base_dir) {
    std::vector<std::string> result;
    for (const auto& entry : std::experimental::filesystem::directory_iterator(base_dir)) {
        result.push_back(entry.path());
    }
    std::sort(result.begin(), result.end());
    return result;
}

void PrintFrame(const std::vector<uint8_t> data) {
    constexpr int32_t kStride = kImageWidth * kColorComponent;
    constexpr int32_t kSubsample = 2;
    int result = 0;
    setupterm( NULL, STDOUT_FILENO, &result );
#pragma unroll
    for (int j = 0; j < kImageHeight; j += kSubsample) {
#pragma unroll
        for (int i = 0; i < kStride; i += kColorComponent * kSubsample) {
            std::cout << (data[j * kStride + i] + data[j * kStride + i + 1] + data[j * kStride + i + 2] != 0);
        }
        std::cout << std::endl;
    }
    usleep((int32_t) (0.03333333333 * 1000000));
}


}  // namespace

int main(int argc, char ** argv) {
    constexpr char kPPMFileDir[] = "/home/chenfs/workspace/bad_apple/frames";
    const std::vector<std::string> all_filelist = GetListOfFiles(kPPMFileDir);
    std::vector<std::vector<uint8_t>> all_data;
    all_data.resize(all_filelist.size());

    for (int i = 0; i < all_filelist.size(); i++) {
        all_data[i] = readPPM(all_filelist[i]);
    }

    for (const auto& frame : all_data) {
        PrintFrame(frame);
    }

}
