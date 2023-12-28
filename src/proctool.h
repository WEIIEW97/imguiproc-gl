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

#ifndef IMGPROC_OPENGL_PROCTOOL_H
#define IMGPROC_OPENGL_PROCTOOL_H

#include "proc.h"
#include <iomanip>
#include <utility>

namespace nvpimgproc::handle {
  class Handler : public Impl {
  public:
    std::vector<std::string> filenames_;
    Handler(std::string in_dir, std::string out_dir)
        : Impl(std::move(in_dir), std::move(out_dir)) {
      retrieve_filenames();
      create_directory();
    }
    ~Handler() = default;

  public:
    void process_img2yuv(std::string& order, std::string& left_marker,
                         std::string& right_marker);
    void process_img_resize(float w_alpha, float h_alpha);
    void process_img_resize(float alpha);
    void process_img_flip(FLIP direction);
    void process_disp2depth(int w, int h, float focal, float baseline);
    void process_colormap(int w, int h, bool is_depth, int minscope,
                          int maxscope);
    void process_rename(std::string pos);
    void process_bin2rgb(int w, int h);
    void process_rgb2bin(bool is_rgb);

  private:
    void retrieve_filenames();
    void create_directory();
  };
} // namespace nvpimgproc::handle

#endif // IMGPROC_OPENGL_PROCTOOL_H
