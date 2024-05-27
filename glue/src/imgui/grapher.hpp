#pragma once

#include <implot.h>

#include <concepts>
#include <glue/types.hpp>
#include <vector>

namespace glue::imgui {
template <typename T>
concept CGraphable = requires(const T& t, std::size_t i) {
  { t.count() } -> std::convertible_to<std::size_t>;
  { t[i].data } -> std::convertible_to<f64>;
  { t[i].time } -> std::convertible_to<f64>;
};

class CustomTicks {
 public:
  CustomTicks(std::initializer_list<std::tuple<double, const char*>> values) {
    std::transform(std::begin(values), std::end(values),
                   std::back_inserter(ys_),
                   [](const auto& value) { return std::get<0>(value); });
    std::transform(std::begin(values), std::end(values),
                   std::back_inserter(labels_),
                   [](const auto& value) { return std::get<1>(value); });
  }

  const double* ys() const { return ys_.data(); }
  const char* const* labels() const { return labels_.data(); }
  std::size_t count() const { return ys_.size(); }

 private:
  std::vector<double> ys_;
  std::vector<const char*> labels_;
};

class GraphInterface {
 public:
  GraphInterface(std::vector<f64>& xs_buffer,
                 std::vector<f64>& ys_buffer) noexcept
      : xs_buffer_{xs_buffer}, ys_buffer_{ys_buffer} {}

  template <CGraphable T>
  void draw(const char* name, const T& graphable) {
    fill_buffers(graphable);
    ImPlot::PlotLine(name, xs_buffer_.data(), ys_buffer_.data(),
                     graphable.count());
  }

  template <CGraphable T>
  void draw_filled(const char* name, const T& graphable) {
    fill_buffers(graphable);
    ImPlot::PlotLine(name, xs_buffer_.data(), ys_buffer_.data(),
                     graphable.count());

    ImPlot::PushStyleVar(ImPlotStyleVar_FillAlpha, 0.4f);
    ImPlot::PlotShaded(name, xs_buffer_.data(), ys_buffer_.data(),
                       graphable.count());
    ImPlot::PopStyleVar();
  }

 private:
  std::vector<f64>& xs_buffer_;
  std::vector<f64>& ys_buffer_;

  template <CGraphable T>
  void fill_buffers(const T& graphable) {
    const auto count = graphable.count();
    if (ys_buffer_.size() < count) {
      ys_buffer_.resize(count);
    }
    if (xs_buffer_.size() < count) {
      xs_buffer_.resize(count);
    }

    for (std::size_t i = 0; i < count; ++i) {
      const auto& entry = graphable[i];
      ys_buffer_[i] = entry.data;
      xs_buffer_[i] = entry.time;
    }
  }
};

class Grapher {
 public:
  template <std::invocable<GraphInterface&> Fn>
  void draw_graph(const char* id, f64 log_window, Fn user_draw_fn,
                  CustomTicks ticks = {}) {
    static f64 y_axis = 0.0;

    if (ImPlot::BeginPlot(id, {-1, 140},
                          ImPlotFlags_NoFrame | ImPlotFlags_NoBoxSelect |
                              ImPlotFlags_NoTitle | ImPlotFlags_NoMenus |
                              ImPlotFlags_NoMouseText)) {
      ImPlot::SetupAxes("s", "ms",
                        ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels |
                            ImPlotAxisFlags_AutoFit,
                        ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_LockMin);
      ImPlot::SetupAxisFormat(ImAxis_Y1, "%g ms");

      ImPlot::SetupAxisLinks(ImAxis_Y1, nullptr, &y_axis);

      ImPlot::SetupAxesLimits(0.0, log_window, 0.0, 1000.0 / 30.0);
      ImPlot::SetupLegend(ImPlotLocation_South, ImPlotLegendFlags_Horizontal |
                                                    ImPlotLegendFlags_Outside);

      if (ticks.count() > 0) {
        ImPlot::SetupAxis(ImAxis_Y2, nullptr,
                          ImPlotAxisFlags_AuxDefault | ImPlotAxisFlags_NoMenus);
        ImPlot::SetupAxisTicks(ImAxis_Y2, ticks.ys(), ticks.count(),
                               ticks.labels(), false);
        ImPlot::SetupAxisLimits(ImAxis_Y2, 0.0, y_axis, ImPlotCond_Always);
      }

      ImPlot::SetAxes(ImAxis_X1, ImAxis_Y1);
      GraphInterface graph_interface{xs_buffer_, ys_buffer_};
      user_draw_fn(graph_interface);

      ImPlot::EndPlot();
    }
  }

 private:
  std::vector<f64> xs_buffer_;
  std::vector<f64> ys_buffer_;
};
}  // namespace glue::imgui