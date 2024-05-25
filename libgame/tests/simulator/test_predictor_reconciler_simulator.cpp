#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <glue/simulator/predictor_reconciler_simulator.hpp>
#include <glue/types.hpp>

using namespace glue::simulator;
using namespace glue;

class PredictorReconcilerTests : public ::testing::Test {
 public:
  auto create_instance(f64 timestep = 1.0 / 60.0, f64 buffer_time = 0.200) {
    return PredictorReconcilerSimulator{
        nullptr, nullptr, {}, timestep, buffer_time};
  }
};

TEST_F(PredictorReconcilerTests, When60TPSAndBuffer200Ms_Buffer12Frames) {
  auto sim = create_instance(1.0 / 60.0, 0.200);
  EXPECT_EQ(sim.buffer_frames(), 12);
}

TEST_F(PredictorReconcilerTests, When60TPSAndBuffer130Ms_Buffer8Frames) {
  auto sim = create_instance(1.0 / 60.0, 0.130);
  EXPECT_EQ(sim.buffer_frames(), 8);
}

TEST_F(PredictorReconcilerTests,
       When60TPSAndBufferLessThanTwoFrames_Buffer2Frames) {
  auto sim = create_instance(1.0 / 60.0, 0.01);
  EXPECT_EQ(sim.buffer_frames(), 2);
}