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

#ifndef IMGPROC_OPENGL_UTILS_H
#define IMGPROC_OPENGL_UTILS_H

#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>

namespace nvpimgproc {
  bool mat2bin_status(const cv::Mat& src, std::string out_dir);
  void mat2bin(const cv::Mat& src, std::string out_dir);
  void bin2mat(std::string in_dir, cv::Mat& dst);
  cv::Mat rgb2yuv_I420(cv::Mat src);
  void I420_to_NV12(const cv::Mat& src, cv::Mat& dst);
  namespace fileop {
    std::vector<std::string> get_file_names_from_dir(std::string root_dir);
    void create_output_dir(const std::string& out_dir);
  } // namespace fileop
} // namespace nvpimgproc

#endif // IMGPROC_OPENGL_UTILS_H
