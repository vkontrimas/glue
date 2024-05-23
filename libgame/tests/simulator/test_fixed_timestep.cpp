#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <glue/simulator/fixed_timestep.hpp>
#include <glue/types.hpp>

using namespace glue;
using namespace glue::simulator;
using namespace testing;

class MockStepper final {
 public:
  MOCK_METHOD(void, step, (f64));
};
static_assert(CStepper<MockStepper>);

class FixedTimestepTests : public ::testing::Test {
 public:
  constexpr f64 epsilon() const noexcept { return 0.000001; }
  auto create_stepper() { return MockStepper{}; }
};

TEST_F(FixedTimestepTests,
       WhenDeltaTimeLessThanTimestep_TimeToNextStepGoesUpByDeltaTime) {
  constexpr f64 delta_time = 1.0 / 300.0;
  auto stepper = create_stepper();

  FixedTimestep fixed_timestep{1.0 / 60.0};
  EXPECT_NEAR(fixed_timestep.time_to_next_step(), 0.0, epsilon());
  fixed_timestep.update(stepper, delta_time);
  EXPECT_NEAR(fixed_timestep.time_to_next_step(), delta_time, epsilon());
  fixed_timestep.update(stepper, delta_time);
  EXPECT_NEAR(fixed_timestep.time_to_next_step(), delta_time + delta_time,
              epsilon());
}

TEST_F(FixedTimestepTests, WhenDeltaTimeIsOneTimestep_StepsOnce) {
  constexpr f64 delta_time = 1.0 / 60.0;
  constexpr f64 timestep = delta_time;

  auto stepper = create_stepper();
  EXPECT_CALL(stepper, step(DoubleEq(timestep))).Times(1);

  FixedTimestep fixed_timestep{timestep};
  fixed_timestep.update(stepper, delta_time);
  EXPECT_NEAR(fixed_timestep.time_to_next_step(), 0.0, epsilon());
}

TEST_F(FixedTimestepTests, WhenDeltaTimeIsFourTimesteps_StepsFourTimes) {
  constexpr f64 timestep = 1.0 / 60.0;
  constexpr f64 delta_time = timestep * 4.0;

  auto stepper = create_stepper();
  EXPECT_CALL(stepper, step(DoubleEq(timestep))).Times(4);

  FixedTimestep fixed_timestep{timestep};
  fixed_timestep.update(stepper, delta_time);
  EXPECT_NEAR(fixed_timestep.time_to_next_step(), 0.0, epsilon());
}

TEST_F(FixedTimestepTests,
       WhenDeltaTimeExceedsTimestep_TimeToNextStepContainsRemainder) {
  constexpr f64 timestep = 1.0 / 60.0;
  constexpr f64 delta_time = timestep * 1.1;
  constexpr f64 remainder = delta_time - timestep;

  auto stepper = create_stepper();
  EXPECT_CALL(stepper, step(DoubleEq(timestep))).Times(1);

  FixedTimestep fixed_timestep{timestep};
  fixed_timestep.update(stepper, delta_time);
  EXPECT_NEAR(fixed_timestep.time_to_next_step(), remainder, epsilon());
}