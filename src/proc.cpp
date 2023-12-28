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
#include "proc.h"
#include <regex>
#include <utility>

float colormapJet[128][3] = {
    {0.0f, 0.0f, 0.53125f},     {0.0f, 0.0f, 0.5625f},
    {0.0f, 0.0f, 0.59375f},     {0.0f, 0.0f, 0.625f},
    {0.0f, 0.0f, 0.65625f},     {0.0f, 0.0f, 0.6875f},
    {0.0f, 0.0f, 0.71875f},     {0.0f, 0.0f, 0.75f},
    {0.0f, 0.0f, 0.78125f},     {0.0f, 0.0f, 0.8125f},
    {0.0f, 0.0f, 0.84375f},     {0.0f, 0.0f, 0.875f},
    {0.0f, 0.0f, 0.90625f},     {0.0f, 0.0f, 0.9375f},
    {0.0f, 0.0f, 0.96875f},     {0.0f, 0.0f, 1.0f},
    {0.0f, 0.03125f, 1.0f},     {0.0f, 0.0625f, 1.0f},
    {0.0f, 0.09375f, 1.0f},     {0.0f, 0.125f, 1.0f},
    {0.0f, 0.15625f, 1.0f},     {0.0f, 0.1875f, 1.0f},
    {0.0f, 0.21875f, 1.0f},     {0.0f, 0.25f, 1.0f},
    {0.0f, 0.28125f, 1.0f},     {0.0f, 0.3125f, 1.0f},
    {0.0f, 0.34375f, 1.0f},     {0.0f, 0.375f, 1.0f},
    {0.0f, 0.40625f, 1.0f},     {0.0f, 0.4375f, 1.0f},
    {0.0f, 0.46875f, 1.0f},     {0.0f, 0.5f, 1.0f},
    {0.0f, 0.53125f, 1.0f},     {0.0f, 0.5625f, 1.0f},
    {0.0f, 0.59375f, 1.0f},     {0.0f, 0.625f, 1.0f},
    {0.0f, 0.65625f, 1.0f},     {0.0f, 0.6875f, 1.0f},
    {0.0f, 0.71875f, 1.0f},     {0.0f, 0.75f, 1.0f},
    {0.0f, 0.78125f, 1.0f},     {0.0f, 0.8125f, 1.0f},
    {0.0f, 0.84375f, 1.0f},     {0.0f, 0.875f, 1.0f},
    {0.0f, 0.90625f, 1.0f},     {0.0f, 0.9375f, 1.0f},
    {0.0f, 0.96875f, 1.0f},     {0.0f, 1.0f, 1.0f},
    {0.03125f, 1.0f, 0.96875f}, {0.0625f, 1.0f, 0.9375f},
    {0.09375f, 1.0f, 0.90625f}, {0.125f, 1.0f, 0.875f},
    {0.15625f, 1.0f, 0.84375f}, {0.1875f, 1.0f, 0.8125f},
    {0.21875f, 1.0f, 0.78125f}, {0.25f, 1.0f, 0.75f},
    {0.28125f, 1.0f, 0.71875f}, {0.3125f, 1.0f, 0.6875f},
    {0.34375f, 1.0f, 0.65625f}, {0.375f, 1.0f, 0.625f},
    {0.40625f, 1.0f, 0.59375f}, {0.4375f, 1.0f, 0.5625f},
    {0.46875f, 1.0f, 0.53125f}, {0.5f, 1.0f, 0.5f},
    {0.53125f, 1.0f, 0.46875f}, {0.5625f, 1.0f, 0.4375f},
    {0.59375f, 1.0f, 0.40625f}, {0.625f, 1.0f, 0.375f},
    {0.65625f, 1.0f, 0.34375f}, {0.6875f, 1.0f, 0.3125f},
    {0.71875f, 1.0f, 0.28125f}, {0.75f, 1.0f, 0.25f},
    {0.78125f, 1.0f, 0.21875f}, {0.8125f, 1.0f, 0.1875f},
    {0.84375f, 1.0f, 0.15625f}, {0.875f, 1.0f, 0.125f},
    {0.90625f, 1.0f, 0.09375f}, {0.9375f, 1.0f, 0.0625f},
    {0.96875f, 1.0f, 0.03125f}, {1.0f, 1.0f, 0.0f},
    {1.0f, 0.96875f, 0.0f},     {1.0f, 0.9375f, 0.0f},
    {1.0f, 0.90625f, 0.0f},     {1.0f, 0.875f, 0.0f},
    {1.0f, 0.84375f, 0.0f},     {1.0f, 0.8125f, 0.0f},
    {1.0f, 0.78125f, 0.0f},     {1.0f, 0.75f, 0.0f},
    {1.0f, 0.71875f, 0.0f},     {1.0f, 0.6875f, 0.0f},
    {1.0f, 0.65625f, 0.0f},     {1.0f, 0.625f, 0.0f},
    {1.0f, 0.59375f, 0.0f},     {1.0f, 0.5625f, 0.0f},
    {1.0f, 0.53125f, 0.0f},     {1.0f, 0.5f, 0.0f},
    {1.0f, 0.46875f, 0.0f},     {1.0f, 0.4375f, 0.0f},
    {1.0f, 0.40625f, 0.0f},     {1.0f, 0.375f, 0.0f},
    {1.0f, 0.34375f, 0.0f},     {1.0f, 0.3125f, 0.0f},
    {1.0f, 0.28125f, 0.0f},     {1.0f, 0.25f, 0.0f},
    {1.0f, 0.21875f, 0.0f},     {1.0f, 0.1875f, 0.0f},
    {1.0f, 0.15625f, 0.0f},     {1.0f, 0.125f, 0.0f},
    {1.0f, 0.09375f, 0.0f},     {1.0f, 0.0625f, 0.0f},
    {1.0f, 0.03125f, 0.0f},     {1.0f, 0.0f, 0.0f},
    {0.96875f, 0.0f, 0.0f},     {0.9375f, 0.0f, 0.0f},
    {0.90625f, 0.0f, 0.0f},     {0.875f, 0.0f, 0.0f},
    {0.84375f, 0.0f, 0.0f},     {0.8125f, 0.0f, 0.0f},
    {0.78125f, 0.0f, 0.0f},     {0.75f, 0.0f, 0.0f},
    {0.71875f, 0.0f, 0.0f},     {0.6875f, 0.0f, 0.0f},
    {0.65625f, 0.0f, 0.0f},     {0.625f, 0.0f, 0.0f},
    {0.59375f, 0.0f, 0.0f},     {0.5625f, 0.0f, 0.0f},
    {0.53125f, 0.0f, 0.0f},     {0.5f, 0.0f, 0.0f}};

namespace nvpimgproc {
  void Impl::set_image_name(std::string img_name) {
    img_name_ = std::move(img_name);
  }

  void Impl::imread() { image_ = cv::imread(in_dir_ + img_name_); }

  void Impl::img2yuv_y_channel_only(int index, const std::string& order) {
    cv::Mat channels[3];
    cv::Mat y_chn;
    cv::split(image_, channels);
    y_chn = channels[2];

    char buffer[64];
    if (order == "L") {
      sprintf(buffer, "rectL_%04d.yuv", index);
    } else if (order == "R") {
      sprintf(buffer, "rectR_%04d.yuv", index);
    }
    std::string full_dir = out_dir_ + buffer;
    bool status = mat2bin_status(y_chn, full_dir);
    if (status) {
      printf("Conversion succeeded!\n");
    } else {
      printf("Conversion failed!\n");
    }
  }

  cv::Mat Impl::img_resize(const cv::Mat& src, float w_alpha, float h_alpha) {
    cv::Mat dst;
    cv::resize(src, dst, cv::Size(src.cols * w_alpha, src.rows * h_alpha), 0, 0,
               cv::INTER_LINEAR);
    return dst;
  }

  cv::Mat Impl::img_resize(const cv::Mat& src, float alpha) {
    cv::Mat dst;
    cv::resize(src, dst, cv::Size(src.cols * alpha, src.rows * alpha), 0, 0,
               cv::INTER_LINEAR);
    return dst;
  }

  void Impl::img_resize(float w_alpha, float h_alpha) {
    cv::Mat img_out;
    cv::resize(image_, img_out,
               cv::Size(image_.cols * h_alpha, image_.rows * w_alpha), 0, 0,
               cv::INTER_LINEAR);
    auto found = img_name_.find_last_of(".");
    std::string marker = img_name_.substr(0, found);
    char buffer[64];
    sprintf(buffer, "%s.bmp", marker.c_str());
    std::string full_dir = out_dir_ + buffer;
    cv::imwrite(full_dir, img_out);
  }

  void Impl::img_resize(float alpha) {
    cv::Mat img_out;
    cv::resize(image_, img_out,
               cv::Size(image_.cols * alpha, image_.rows * alpha), 0, 0,
               cv::INTER_LINEAR);
    auto found = img_name_.find_last_of(".");
    std::string marker = img_name_.substr(0, found);
    char buffer[64];
    sprintf(buffer, "%s.bmp", marker.c_str());
    std::string full_dir = out_dir_ + buffer;
    cv::imwrite(full_dir, img_out);
  }

  cv::Mat Impl::img_flip(const cv::Mat& src, nvpimgproc::FLIP direction) {
    cv::Mat dst;
    if (direction == X_AXIS) {
      printf("Flipping image along x-axis!\n");
      cv::flip(src, dst, X_AXIS);
    } else if (direction == Y_AXIS) {
      printf("Flipping image along y-axis!\n");
      cv::flip(src, dst, Y_AXIS);
    } else if (direction == XY_AXIS) {
      printf("Flipping image along xy-axis!\n");
      cv::flip(src, dst, XY_AXIS);
    } else {
      printf("Axis flipping direction is not supported!\n");
    }
    return dst;
  }

  void Impl::img_flip(nvpimgproc::FLIP direction) {
    cv::Mat imgfp;
    if (direction == X_AXIS) {
      printf("Flipping image along x-axis!\n");
      cv::flip(image_, imgfp, X_AXIS);
    } else if (direction == Y_AXIS) {
      printf("Flipping image along y-axis!\n");
      cv::flip(image_, imgfp, Y_AXIS);
    } else if (direction == XY_AXIS) {
      printf("Flipping image along xy-axis!\n");
      cv::flip(image_, imgfp, XY_AXIS);
    } else {
      printf("Axis flipping direction is not supported!\n");
    }
    std::string full_dir = out_dir_ + img_name_;
    cv::imwrite(full_dir, imgfp);
  }

  void Impl::disp2depth(int w, int h, float focal, float baseline) {
    cv::Mat disp = cv::Mat::zeros(cv::Size(w, h), CV_16UC1);
    FILE* fp = fopen((in_dir_ + img_name_).c_str(), "rb");
    fread(disp.data, sizeof(uint16_t), w * h, fp);
    fclose(fp);
    const float fb = focal * baseline;
    cv::Mat depth = cv::Mat::zeros(cv::Size(w, h), CV_32FC1);
    for (int i = 0; i < disp.rows; ++i) {
      auto* disp_ptr = disp.ptr<uint16_t>(i);
      auto* depth_ptr = depth.ptr<float>(i);
      for (int j = 0; j < disp.cols; ++j) {
        if (disp_ptr[j] != 0) {
          depth_ptr[j] = fb / (float)disp_ptr[j] * pow(2, 6);
        }
      }
    }
    std::string name = img_name_;
    /*boost::replace_all(name, "yuv", "raw");*/
    name = std::regex_replace(name, std::regex("yuv"), "raw");

    depth.convertTo(depth, CV_16UC1, 1, 0);
    FILE* f = fopen((out_dir_ + name).c_str(), "wb");
    fwrite(depth.data, sizeof(uint16_t), w * h, f);
    fclose(f);
  }

  void Impl::set_colormap_params(int num_bar_width, int color_bar_width,
                                 int vline) {
    num_bar_width_ = num_bar_width;
    color_bar_width_ = color_bar_width;
    vline_ = vline;
  }

  cv::Mat Impl::colormap_kernel(cv::Mat src, int minscope, int maxscope) {
    cv::Mat color = cv::Mat(
        src.size(), CV_8UC3,
        cv::Scalar(colormapJet[0][2], colormapJet[0][1], colormapJet[0][0]) *
            255.0f);
    cv::Mat window_ = cv::Mat::zeros(
        cv::Size(src.cols + num_bar_width_ + color_bar_width_ + vline_,
                 src.rows),
        CV_8UC3);

    double min_range, max_range;
    cv::Point min_loc, max_loc;
    cv::minMaxLoc(src, &min_range, &max_range, &min_loc, &max_loc);
    // determine the range by self
    if (maxscope == 0) {
      minscope = min_range;
      maxscope = max_range;
    }
    for (int i = 0; i < color.rows; ++i) {
      const uint16_t* src_ptr = src.ptr<uint16_t>(i);
      uchar* pixel = color.ptr<uchar>(i);
      for (int j = 0; j < color.cols; ++j) {
        if (src_ptr[j] != 0) {
          uint16_t tmp = src_ptr[j];
          int idx = fminf((float)src_ptr[j] - (float)minscope,
                          (float)maxscope - (float)minscope) /
                    ((float)maxscope - (float)minscope) * 127.0f;
          // idx = 127 - idx;

          pixel[0] = colormapJet[idx][2] * 255.0f;
          pixel[1] = colormapJet[idx][1] * 255.0f;
          pixel[2] = colormapJet[idx][0] * 255.0f;
        }
        pixel += 3;
      }
    }

    color.copyTo(window_(cv::Rect(0, 0, color.cols, color.rows)));
    // scaling bar
    cv::Mat bar_ = cv::Mat::zeros(cv::Size(num_bar_width_, src.rows), CV_8UC3);

    int interval = (maxscope - minscope) / 10;
    for (int i = minscope; i < maxscope; ++i) {
      int j = i * src.rows / (maxscope - minscope);
      if (i % interval == 0) {
        cv::putText(bar_, std::to_string(i), cv::Point(5, bar_.rows - j - 5),
                    cv::FONT_HERSHEY_DUPLEX, 0.4, cv::Scalar(255, 255, 255));
      }
    }
    // color bar
    cv::Mat color_bar =
        cv::Mat::ones(cv::Size(color_bar_width_, src.rows), CV_8UC3) * 255;
    for (int i = 0; i < color_bar.rows; ++i) {
      uchar* pixel = color_bar.ptr<uchar>(i);
      int v = 127 - ((float)i / color_bar.rows) * 127;
      // int v = ((float)i / color_bar.rows) * 127;
      for (int j = 0; j < color_bar_width_; ++j) {
        pixel[0] = colormapJet[v][2] * 255.0f;
        pixel[1] = colormapJet[v][1] * 255.0f;
        pixel[2] = colormapJet[v][0] * 255.0f;
        pixel += 3;
      }
    }

    // concate scaling bar and color_bar
    bar_.copyTo(window_(cv::Rect(src.cols + vline_ + color_bar_width_, 0,
                                 num_bar_width_, src.rows)));
    color_bar.copyTo(
        window_(cv::Rect(src.cols + vline_, 0, color_bar_width_, src.rows)));
    cv::Mat dst = window_.clone();

    return dst;
  }

  void Impl::colormap(int w, int h, int index, bool is_depth, int minscope,
                      int maxscope) {
    cv::Mat disp = cv::Mat::zeros(cv::Size(w, h), CV_16UC1);
    size_t found = img_name_.find_last_of(".");
    std::string marker = img_name_.substr(found + 1);
    if (marker == "png" || marker == "bmp" || marker == "jpg") {
      // read uint16 img by imread
      disp = cv::imread((in_dir_ + img_name_), cv::IMREAD_ANYDEPTH);
    } else {
      // bin2mat((in_dir + img_name), disp);
      FILE* fp = fopen((in_dir_ + img_name_).c_str(), "rb");
      fread(disp.data, sizeof(uint16_t), w * h, fp);
      fclose(fp);
    }

    cv::Mat Disp = cv::Mat::zeros(cv::Size(w, h), CV_16UC1);
    if (!is_depth) {
      for (int i = 0; i < disp.rows; ++i) {
        uint16_t* disp_ptr = disp.ptr<uint16_t>(i);
        uint16_t* Disp_ptr = Disp.ptr<uint16_t>(i);
        for (int j = 0; j < disp.cols; ++j) {
          Disp_ptr[j] = (uint16_t)disp_ptr[j] / pow(2, 6);
        }
      }
    } else {
      disp.copyTo(Disp);
    }

    cv::Mat dst = colormap_kernel(Disp, minscope, maxscope);
    char buffer[64];
    // sprintf(buffer, "color_%s.png", key_.c_str());
    sprintf(buffer, "color_%04d.png", index);
    std::string full_dir = out_dir_ + buffer;
    cv::imwrite(full_dir, dst);
  }

  void Impl::rename(std::string pos, int index) {
    char buffer[64];
    if (pos == "L") {
      sprintf(buffer, "left-%04d.bmp", index);
      std::string full_dir = out_dir_ + buffer;
      cv::imwrite(full_dir, image_);
    } else if (pos == "R") {
      sprintf(buffer, "right-%04d.bmp", index);
      std::string full_dir = out_dir_ + buffer;
      cv::imwrite(full_dir, image_);
    }
  }

  void Impl::bin2rgb(int w, int h) {
    size_t found = img_name_.find_last_of(".");
    std::string marker = img_name_.substr(found + 1);
    if (marker == "raw" || marker == "yuv" || marker == "bin") {
      std::ifstream file((in_dir_ + img_name_),
                         std::ifstream::binary | std::ifstream::in);
      assert(file.is_open());

      int len;
      // get size of stream
      file.seekg(0, std::ios::end);
      len = file.tellg();

      // return the beginning of stream
      file.seekg(0, std::ios::beg);
      file.close();
      cv::Mat img = cv::Mat::zeros(cv::Size(w, h), CV_8UC1);
      if (len == w * h) {
        bin2mat((in_dir_ + img_name_), img);
      } else if (len == w * h * 2 || len == w * h * 4) {
        // file length might be w * h * 4 since the fuckin HiK
        // stupid duplicated storage.
        img.convertTo(img, CV_16UC1);
        FILE* fp = fopen((in_dir_ + img_name_).c_str(), "rb");
        fread(img.data, sizeof(uint16_t), w * h, fp);
        fclose(fp);
      } else if (len == w * h * 3 / 2) {
        cv::Mat yuv420sp =
            cv::Mat::zeros(cv::Size(w, static_cast<int>(h * 3 / 2)), CV_8UC1);
        FILE* fp = fopen((in_dir_ + img_name_).c_str(), "rb");
        fread(yuv420sp.data, sizeof(uint8_t), w * h * 3 / 2, fp);
        fclose(fp);
        cv::cvtColor(yuv420sp, img, cv::COLOR_YUV420sp2RGB);
      }

      std::string img_name_;
      img_name_ = std::regex_replace(img_name_, std::regex(marker), "png");
      cv::imwrite((out_dir_ + img_name_), img);
    }
  }

  void Impl::rgb2bin(bool is_rgb) {
    size_t pos = img_name_.find_last_of(".");
    std::string suffix = img_name_.substr(pos + 1);
    std::string name_replaced =
        std::regex_replace(img_name_, std::regex(suffix), "yuv");
    if (is_rgb) {
      cv::Mat img = cv::imread(in_dir_ + img_name_);
      cv::Mat i420;
      cv::cvtColor(img, i420, cv::COLOR_RGB2YUV_I420);
      mat2bin(i420, out_dir_ + name_replaced);
    } else {
      name_replaced = std::regex_replace(img_name_, std::regex(suffix), "raw");
      cv::Mat img = cv::imread(in_dir_ + img_name_, cv::IMREAD_UNCHANGED);
      if (img.channels() == 3) {
        cv::cvtColor(img, img, cv::COLOR_RGB2GRAY);
      }
      mat2bin(img, out_dir_ + name_replaced);
    }
  }
  void Impl::change_input_directory(std::string new_in_dir) {
    in_dir_ = std::move(new_in_dir);
  }

  void Impl::change_output_directory(std::string new_out_dir) {
    out_dir_ = std::move(new_out_dir);
  }

  void Impl::change_in_out_directory(std::string new_in_dir,
                                     std::string new_out_dir) {
    in_dir_ = std::move(new_in_dir);
    out_dir_ = std::move(new_out_dir);
  }
} // namespace nvpimgproc
