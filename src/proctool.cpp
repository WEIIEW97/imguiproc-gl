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
#include "proctool.h"

#define HANDLER_LOOP_BEGIN                                                     \
  for (auto& name : filenames_) {                                              \
    set_image_name(name);                                                      \
    imread();

#define HANDLER_LOOP_END }

namespace nvpimgproc {
  namespace handle {
    void Handler::retrieve_filenames() {
      filenames_ = nvpimgproc::fileop::get_file_names_from_dir(in_dir_);
      std::sort(filenames_.begin(), filenames_.end());
    }

    void Handler::create_directory() {
      nvpimgproc::fileop::create_output_dir(out_dir_);
    }

    void Handler::process_img2yuv(std::string& order, std::string& left_marker,
                                  std::string& right_marker) {
      if (order == "L") {
        int j = 0;
        for (auto& name : filenames_) {
          auto pos = name.find(left_marker);
          if (pos != name.npos) {
            set_image_name(name);
            imread();
            img2yuv_y_channel_only(j, order);
          }
        }
      } else if (order == "R") {
        int j = 0;
        for (const auto& name : filenames_) {
          auto pos = name.find(right_marker);
          if (pos != name.npos) {
            set_image_name(name);
            imread();
            img2yuv_y_channel_only(j, order);
          }
        }
      }
    }

    void Handler::process_img_resize(float w_alpha, float h_alpha) {
      HANDLER_LOOP_BEGIN
      img_resize(w_alpha, h_alpha);
      HANDLER_LOOP_END
    }

    void Handler::process_img_resize(float alpha) {
      HANDLER_LOOP_BEGIN
      img_resize(alpha);
      HANDLER_LOOP_END
    }

    void Handler::process_img_flip(nvpimgproc::FLIP direction) {
      HANDLER_LOOP_BEGIN
      img_flip(direction);
      HANDLER_LOOP_END
    }

    void Handler::process_disp2depth(int w, int h, float focal,
                                     float baseline) {
      HANDLER_LOOP_BEGIN
      disp2depth(w, h, focal, baseline);
      HANDLER_LOOP_END
    }

    void Handler::process_colormap(int w, int h, bool is_depth, int minscope,
                                   int maxscope) {
      int j = 0;
      HANDLER_LOOP_BEGIN
      colormap(w, h, j, is_depth, minscope, maxscope);
      ++j;
      HANDLER_LOOP_END
    }

    void Handler::process_rename(std::string pos) {
      int j = 0;
      HANDLER_LOOP_BEGIN
      rename(pos, j);
      ++j;
      HANDLER_LOOP_END
    }

    void Handler::process_rgb2bin(bool is_rgb) {
      HANDLER_LOOP_BEGIN
      rgb2bin(is_rgb);
      HANDLER_LOOP_END
    }

    void Handler::process_bin2rgb(int w, int h) {
      HANDLER_LOOP_BEGIN
      bin2rgb(w, h);
      HANDLER_LOOP_END
    }
  } // namespace handle
} // namespace nvpimgproc
