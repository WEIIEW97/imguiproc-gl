/*
 * Copyright (c) 2022-2023, William Wei. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "utils.h"

namespace fs = std::filesystem;
namespace nvpimgproc {
  bool mat2bin_status(const cv::Mat& src, std::string out_dir) {
    std::ofstream fp;
    fp.open(out_dir, std::ios_base::binary);
    if (!fp.is_open() || src.empty()) {
      return false;
    }
    fp.write((const char*)(src.data), src.elemSize() * src.total());
    fp.close();
    return true;
  }

  void mat2bin(const cv::Mat& src, std::string out_dir) {
    std::ofstream fp;
    fp.open(out_dir, std::ios_base::binary);
    if (!fp.is_open() || src.empty()) {
      std::cerr << "Writing binary file failed." << std::endl;
    }
    fp.write((const char*)src.data, src.elemSize() * src.total());
    fp.close();
  }

  void bin2mat(std::string in_dir, cv::Mat& dst) {
    std::ifstream fp;
    fp.open(in_dir, std::ios_base::binary);
    if (!fp.is_open()) {
      std::cerr << "Writing binary file failed." << std::endl;
    }
    fp.read((char*)dst.data, dst.elemSize() * dst.total());
    fp.close();
  }

  cv::Mat rgb2yuv_I420(cv::Mat src) {
    cv::Mat dst_I420;
    cv::cvtColor(src, dst_I420, cv::COLOR_BGR2YUV_I420);
    return dst_I420;
  }

  void I420_to_NV12(const cv::Mat& src, cv::Mat& dst) {
    int width = src.cols;
    int height = src.rows * 2 / 3;
    int stride = (int)src.step[0];

    src.copyTo(dst);

    // Y Channel
    //  YYYYYYYYYYYYYYYY
    //  YYYYYYYYYYYYYYYY
    //  YYYYYYYYYYYYYYYY
    //  YYYYYYYYYYYYYYYY
    //  YYYYYYYYYYYYYYYY
    //  YYYYYYYYYYYYYYYY

    // V Input channel
    //  VVVVVVVV
    //  VVVVVVVV
    //  VVVVVVVV
    cv::Mat v =
        cv::Mat(cv::Size(width / 2, height / 2), CV_8UC1,
                (unsigned char*)src.data + stride * height,
                stride / 2); // Input V color channel (in YV12 V is above U).

    // U Input channel
    //  UUUUUUUU
    //  UUUUUUUU
    //  UUUUUUUU
    cv::Mat u =
        cv::Mat(cv::Size(width / 2, height / 2), CV_8UC1,
                (unsigned char*)src.data + stride * height +
                    (stride / 2) * (height / 2),
                stride / 2); // Input V color channel (in YV12 U is below V).

    for (int row = 0; row < height / 2; row++) {
      for (int col = 0; col < width / 2; col++) {
        dst.ptr<uchar>(height + row)[2 * col] = u.ptr<uchar>(row)[col];
        dst.ptr<uchar>(height + row)[2 * col + 1] = v.ptr<uchar>(row)[col];
      }
    }
  }

  namespace fileop {
    std::vector<std::string> get_file_names_from_dir(std::string root_dir) {
      std::vector<std::string> file_names;
      fs::path rootdir(root_dir);
      for (auto i = fs::directory_iterator(rootdir);
           i != fs::directory_iterator(); i++) {
        if (!fs::is_directory(i->path())) {
          std::string img_name = i->path().filename().string();
          file_names.emplace_back(img_name);
        }
      }
      return file_names;
    }

    void create_output_dir(const std::string& out_dir) {
      fs::path outdir(out_dir);
      if (!fs::exists(outdir)) {
        fs::create_directories(outdir);
      }
    }
  } // namespace fileop
} // namespace nvpimgproc