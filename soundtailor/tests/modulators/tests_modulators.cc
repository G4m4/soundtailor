/// @file tests_modulators.cc
/// @brief SoundTailor modulators common tests
/// @author gm
/// @copyright gm 2014
///
/// This file is part of SoundTailor
///
/// SoundTailor is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// SoundTailor is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with SoundTailor.  If not, see <http://www.gnu.org/licenses/>.

#include "soundtailor/tests/modulators/tests_modulators_fixture.h"

#include "soundtailor/src/modulators/adsd.h"

using soundtailor::modulators::Adsd;

/// @brief All tested types
typedef ::testing::Types<Adsd> ModulatorTypes;

TYPED_TEST_CASE(Modulator, ModulatorTypes);

/// @brief Generates an envelop, check for its range (must be >= 0)
TYPED_TEST(Modulator, Range) {
  for (unsigned int iterations(0);
       iterations < this->kTestIterations;
       ++iterations) {
    IGNORE(iterations);

    TypeParam generator;
    generator.SetParameters(this->kAttack,
                            this->kDecay,
                            this->kDecay,
                            this->kSustainLevel);

    const float kEpsilon(1e-3f);

    generator.TriggerOn();
    unsigned int i(0);
    // Attack / Decay / Sustain
    while (i <= this->kAttack + this->kDecay + this->kSustain) {
      const float sample(generator());
      EXPECT_LE(0.0f - kEpsilon, sample);
      EXPECT_GE(1.0f + kEpsilon, sample);
      i += 1;
    }
    // Release + a little bit after that
    generator.TriggerOff();
    while (i <= this->kAttack
                + this->kDecay
                + this->kSustain
                + this->kDecay
                + this->kTail) {
      const float sample(generator());
      EXPECT_LE(0.0f - kEpsilon, sample);
      EXPECT_GE(1.0f + kEpsilon, sample);
      i += 1;
    }
  }  // iterations?
}

/// @brief Generates an envelop, check for its proper timings:
/// - when in attack samples should be in a continuous upward slope
/// - when in decay samples should be in a continuous downward slope
/// - when in sustain samples should all be equal
/// - when in release samples should be in a continuous downward slope
TYPED_TEST(Modulator, Timings) {
  for (unsigned int iterations(0);
       iterations < this->kTestIterations;
       ++iterations) {
    IGNORE(iterations);

    TypeParam generator;
    generator.SetParameters(this->kAttack,
                            this->kDecay,
                            this->kDecay,
                            this->kSustainLevel);

    generator.TriggerOn();
    std::vector<unsigned int> zero_crossing_indexes;

    // TODO(gm): get rid of that
    Modulator::AdsdFunctor modulators_functor(&generator);
    ZeroCrossing<Modulator::AdsdFunctor> zero_crossing(modulators_functor);
    unsigned int kTriggerOnLength(this->kAttack + this->kDecay + this->kSustain);
    unsigned int kTotalLength(kTriggerOnLength + this->kDecay + this->kTail);
    // A tiny delay occurs due to differentiation and trigger unevenness
    unsigned int kEpsilon(2);
    unsigned int zero_crossing_idx(
      zero_crossing.GetNextZeroCrossing(kTriggerOnLength));
    while (zero_crossing_idx < kTriggerOnLength) {
      zero_crossing_indexes.push_back(zero_crossing_idx);
      zero_crossing_idx = zero_crossing.GetNextZeroCrossing(kTriggerOnLength);
    }
    generator.TriggerOff();
    while (zero_crossing_idx < kTotalLength) {
      zero_crossing_idx = zero_crossing.GetNextZeroCrossing(kTotalLength);
      zero_crossing_indexes.push_back(zero_crossing_idx);
    }
    EXPECT_NEAR(this->kAttack, zero_crossing_indexes[0], kEpsilon);
    EXPECT_NEAR(this->kAttack + this->kDecay,
                zero_crossing_indexes[1],
                kEpsilon);
    EXPECT_NEAR(kTriggerOnLength, zero_crossing_indexes[2], kEpsilon);
    EXPECT_NEAR(kTriggerOnLength + this->kDecay,
                zero_crossing_indexes[3],
                kEpsilon);
  }  // iterations?
}

/// @brief Check timings for long slopes
///
/// With the parameters used here, the slope is still supposed
/// to be > FLT_EPSILON
TYPED_TEST(Modulator, LongTimings) {
  const unsigned int kLongTime(kMaxTime);
  const unsigned int kAttack(kLongTime);
  const unsigned int kDecay(kLongTime);
  const unsigned int kSustain(100);

  TypeParam generator;
  generator.SetParameters(kAttack, kDecay, kDecay, this->kSustainLevel);

  generator.TriggerOn();
  std::vector<unsigned int> zero_crossing_indexes;

  // TODO(gm): get rid of that
  Modulator::AdsdFunctor modulators_functor(&generator);
  ZeroCrossing<Modulator::AdsdFunctor> zero_crossing(modulators_functor);
  unsigned int kTriggerOnLength(kAttack + kDecay + kSustain);
  unsigned int kTotalLength(kTriggerOnLength + kDecay + kTail);
  // A tiny delay occurs due to differentiation and trigger unevenness
  unsigned int kEpsilon(2);
  unsigned int zero_crossing_idx(
    zero_crossing.GetNextZeroCrossing(kTriggerOnLength));
  while (zero_crossing_idx < kTriggerOnLength) {
    zero_crossing_indexes.push_back(zero_crossing_idx);
    zero_crossing_idx = zero_crossing.GetNextZeroCrossing(kTriggerOnLength);
  }
  generator.TriggerOff();
  while (zero_crossing_idx < kTotalLength) {
    zero_crossing_idx = zero_crossing.GetNextZeroCrossing(kTotalLength);
    zero_crossing_indexes.push_back(zero_crossing_idx);
  }
  EXPECT_NEAR(kAttack, zero_crossing_indexes[0], kEpsilon);
  EXPECT_NEAR(kAttack + kDecay, zero_crossing_indexes[1], kEpsilon);
  EXPECT_NEAR(kTriggerOnLength, zero_crossing_indexes[2], kEpsilon);
  EXPECT_NEAR(kTriggerOnLength + kDecay, zero_crossing_indexes[3], kEpsilon);
}

/// @brief Generates an envelop with one or both timing parameters null
TYPED_TEST(Modulator, NullParameters) {
  for (unsigned int iterations(0);
       iterations < this->kTestIterations;
       ++iterations) {
    IGNORE(iterations);

    // Random parameters
    // Each parameter has half a chance to be null
    const unsigned int kAttack(kBoolDistribution(kRandomGenerator)
                               ? this->kAttack : 0);
    const unsigned int kDecay(kBoolDistribution(kRandomGenerator)
                              ? this->kDecay : 0);

    TypeParam generator;
    generator.SetParameters(kAttack, kDecay, kDecay, this->kSustainLevel);

    const float kEpsilon(1e-3f);

    generator.TriggerOn();
    unsigned int i(1);
    float previous(generator());
    while (i <= kAttack) {
      const float sample(generator());
      EXPECT_LE(previous, sample);
      previous = sample;
      i += 1;
    }
    // If the attack is null, then we have to ignore its "click".
    if (0 == kAttack) {
      previous = generator();
    }
    while (i <= kAttack + kDecay) {
      const float sample(generator());
      // TODO(gm): this epsilon should not be here, remove it
      EXPECT_GE(previous + 1e-6f, sample);
      previous = sample;
      i += 1;
    }
    // If the decay is null, then we have to ignore its "click".
    if (0 == kDecay) {
      previous = generator();
    }
    while (i <= kAttack + kDecay + this->kSustain) {
      const float sample(generator());
      // A (really tiny) epsilon is required here for imprecisions
      const float kNearEpsilon(1e-6f);
      EXPECT_NEAR(this->kSustainLevel, sample, kNearEpsilon);
      i += 1;
    }
    generator.TriggerOff();
    while (i < kAttack + kDecay + this->kSustain + kDecay + this->kTail) {
      const float sample(generator());
      EXPECT_LE(0.0f - kEpsilon, sample);
      EXPECT_GE(1.0f + kEpsilon, sample);
      i += 1;
    }
  }  // iterations?
}

/// @brief Generates a "click envelop" - with both timing parameters null
TYPED_TEST(Modulator, Click) {
  for (unsigned int iterations(0);
       iterations < this->kTestIterations;
       ++iterations) {
    IGNORE(iterations);

    const unsigned int kAttack(0);
    const unsigned int kDecay(0);

    TypeParam generator;
    generator.SetParameters(kAttack, kDecay, kDecay, this->kSustainLevel);

    generator.TriggerOn();
    unsigned int i(1);
    // The first sample is always null!
    IGNORE(generator());
    while (i <= this->kSustain) {
      const float sample(generator());
      // A (really tiny) epsilon is required here for imprecisions
      const float kEpsilon(1e-6f);
      EXPECT_NEAR(this->kSustainLevel, sample, kEpsilon);
      i += 1;
    }
    generator.TriggerOff();
    // This sample left is due to the release
    IGNORE(generator());
    while (i <= this->kSustain + this->kTail) {
      const float sample(generator());
      EXPECT_EQ(0.0f, sample);
      i += 1;
    }
  }  // iterations?
}

/// @brief Generates an envelop, check for its "regularity"
/// the more regular it is, the less spiky, the less liable to audio artefacts
/// on the modulated signal
/// Checking regularity is done as follows: since timings are parameterized,
/// it is easy to deduce the maximum theoretical change
/// between two consecutive samples. Nowhere it should be greater!
TYPED_TEST(Modulator, OutRegularity) {
  for (unsigned int iterations(0);
       iterations < this->kTestIterations;
       ++iterations) {
    IGNORE(iterations);

    TypeParam generator;
    generator.SetParameters(this->kAttack,
                            this->kDecay,
                            this->kDecay,
                            this->kSustainLevel);

    generator.TriggerOn();
    unsigned int i(1);
    // A very small epsilon is added for computation/casts imprecisions
    // TODO(gm): this might not be required if all floating point operations
    // were properly understood and managed.
    const double kMaxDelta(1.0 / std::min(kAttack, kDecay) + 1e-7);
    // Envelops should all begin at zero!
    double previous(static_cast<double>(generator()));
    // Checking the whole envelop since clicks may occur anywhere
    while (i < kAttack + kDecay + this->kSustain) {
      const double sample(static_cast<double>(generator()));
      const double diff(std::fabs(sample - previous));
      EXPECT_GE(kMaxDelta, diff);
      previous = sample;
      i += 1;
    }
    generator.TriggerOff();
    while (i < kAttack + kDecay + this->kSustain + kDecay + this->kTail) {
      const double sample(static_cast<double>(generator()));
      const double diff(std::fabs(sample - previous));
      EXPECT_GE(kMaxDelta, diff);
      previous = sample;
      i += 1;
    }
  }  // iterations?
}


/// @brief Generates an envelopm (performance test)
// Here the tested length cannot be longer in release configuration,
// because it would then only test the performance on the envelop tail!
// That's why we are using a "kModulatorPerfIterations", adding iterations
// when in Release.
TYPED_TEST(Modulator, Perf) {
  for (unsigned int iterations(0);
       iterations < this->kPerfIterations;
       ++iterations) {
    IGNORE(iterations);

    TypeParam generator;
    generator.SetParameters(this->kAttack,
                            this->kDecay,
                            this->kDecay,
                            this->kSustainLevel);

    generator.TriggerOn();

    unsigned int sample_idx(0);
    while (sample_idx < this->kAttack + this->kDecay + this->kSustain) {
      const float kCurrent(generator());
      sample_idx += 1;
      // No actual test!
      EXPECT_LE(-1.0f, kCurrent);
    }
    generator.TriggerOff();
    while (sample_idx < this->kModulatorDataPerfSetSize) {
      const float kCurrent(generator());
      sample_idx += 1;
      // No actual test!
      EXPECT_LE(-1.0f, kCurrent);
    }
  }  // iterations?
}
