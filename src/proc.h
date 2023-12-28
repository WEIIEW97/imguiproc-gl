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

#ifndef IMGPROC_OPENGL_PROC_H
#define IMGPROC_OPENGL_PROC_H

#include "utils.h"
#include <opencv2/highgui.hpp>
#include <utility>
#include <vector>

namespace nvpimgproc {

  enum FLIP {
    X_AXIS = 0,
    Y_AXIS = 1,
    XY_AXIS = -1,
  };

  class Impl {
  public:
    std::string in_dir_, out_dir_, img_name_;
    cv::Mat image_;
    Impl(std::string in_dir, std::string out_dir)
        : in_dir_(std::move(in_dir)), out_dir_(std::move(out_dir)) {}
    ~Impl() = default;

  public:
    virtual void change_input_directory(std::string new_in_dir);
    virtual void change_output_directory(std::string new_out_dir);
    virtual void change_in_out_directory(std::string new_in_dir, std::string new_out_dir);
    void set_image_name(std::string img_name);
    void imread();
    void img2yuv_y_channel_only(int index, const std::string& order);
    cv::Mat img_resize(const cv::Mat& src, float w_alpha, float h_alpha);
    cv::Mat img_resize(const cv::Mat& src, float alpha);
    void img_resize(float w_alpha, float h_alpha);
    void img_resize(float alpha);
    cv::Mat img_flip(const cv::Mat& src, FLIP direction);
    void img_flip(FLIP direction);
    void disp2depth(int w, int h, float focal, float baseline);
    void colormap(int w, int h, int index, bool is_depth, int minscope,
                  int maxscope);
    void rename(std::string pos, int index);
    void bin2rgb(int w, int h);
    void rgb2bin(bool is_rgb);
    void set_colormap_params(int num_bar_width, int color_bar_width, int vline);

  private:
    cv::Mat colormap_kernel(cv::Mat src, int minscope, int maxscope);

  private:
    int num_bar_width_ = 55;
    int color_bar_width_ = 35;
    int vline_ = 35;
  };
} // namespace nvpimgproc

#endif // IMGPROC_OPENGL_PROC_H
