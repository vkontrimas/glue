#pragma once

#include <imgui.h>

#include <initializer_list>
#include <tuple>

namespace glue::imgui {
template <typename T>
class OptionSlider {
 public:
  OptionSlider(std::initializer_list<std::pair<T, const char*>> values)
      : values_{values} {}

  const T& selected_value() const { return values_[index_].first; }
  const char* selected_label() const { return values_[index_].second; }
  bool last_value_selected() const { return index_ == values_.size() - 1; }

  void draw(const char* label) {
    ImGui::SliderInt(label, &index_, 0, static_cast<i32>(values_.size() - 1),
                     values_[index_].second,
                     ImGuiSliderFlags_NoInput | ImGuiSliderFlags_AlwaysClamp);
  }

 private:
  std::vector<std::pair<T, const char*>> values_;
  i32 index_ = 0;
};
}  // namespace glue::imgui