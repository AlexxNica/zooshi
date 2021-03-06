// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "full_screen_fader.h"

#include "fplbase/material.h"
#include "fplbase/mesh.h"
#include "fplbase/renderer.h"
#include "fplbase/shader.h"

namespace fpl {
namespace zooshi {

FullScreenFader::FullScreenFader()
    : current_fade_time_(0),
      total_fade_time_(0),
      end_fade_time_(0),
      material_(NULL),
      shader_(NULL),
      opaque_(false) {}

void FullScreenFader::Init(fplbase::Material* material,
                           fplbase::Shader* shader) {
  material_ = material;
  shader_ = shader;
}

// Starts the fade.
void FullScreenFader::Start(corgi::WorldTime fade_time,
                            const mathfu::vec3& color, FadeType fade_type,
                            const mathfu::vec3& bottom_left,
                            const mathfu::vec3& top_right) {
  assert(material_ && shader_);
  current_fade_time_ = fade_type == kFadeIn ? fade_time : 0;
  total_fade_time_ = fade_type == kFadeIn || fade_type == kFadeOut ?
    2 * fade_time : fade_time;
  end_fade_time_ = fade_type == kFadeOut ? fade_time : total_fade_time_;
  color_ = color;
  bottom_left_ = bottom_left;
  top_right_ = top_right;
  opaque_ = false;
}

// Update the fade color returning true on the frame the overlay
// is opaque.
bool FullScreenFader::AdvanceFrame(int delta_time) {
  if (Finished()) {
    return false;
  }
  current_fade_time_ += delta_time;
  bool opaque = current_fade_time_ > (total_fade_time_ / 2);
  bool turned_opaque_this_frame = !opaque_ && opaque;
  opaque_ = opaque;
  return turned_opaque_this_frame;
}

// Renders the fade overlay.
void FullScreenFader::Render(fplbase::Renderer* renderer) {
  float t = std::min(static_cast<float>(std::min(current_fade_time_,
                                                 end_fade_time_)) /
                         static_cast<float>(total_fade_time_), 1.0f);
  float alpha = sin(t * static_cast<float>(M_PI));
  // Render the overlay in front on the screen.
  renderer->set_color(mathfu::vec4(color_, alpha));
  material_->Set(*renderer);
  shader_->Set(*renderer);
  // Clear depth buffer to prevent z-fight issues.
  renderer->ClearDepthBuffer();
  fplbase::Mesh::RenderAAQuadAlongX(bottom_left_, top_right_);
}

// Returns true when the fade is complete (overlay is transparent).
bool FullScreenFader::Finished() const {
  return current_fade_time_ >= end_fade_time_;
}

// Get the fraction (0..1) elapsed through the fader's fade time.
float FullScreenFader::GetOffset() const {
  return std::min(static_cast<float>(std::min(current_fade_time_,
                                              end_fade_time_)) /
                         static_cast<float>(end_fade_time_), 1.0f);
}

}  // namespace zooshi
}  // namespace fpl
