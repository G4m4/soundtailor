/// @file tests_adsd.cc
/// @brief ADSD envelop generator specific tests
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

#include "soundtailor/tests/tests.h"

#include "soundtailor/src/generators/generators_common.h"
#include "soundtailor/src/modulators/adsd.h"

// Using declarations for tested generator
using soundtailor::modulators::Adsd;
// Using declaration for differentiator
using soundtailor::generators::Differentiator;

/// @brief Time parameters random generator
static std::uniform_int_distribution<unsigned int> kTimeDistribution(kMinTime,
                                                                     kMaxTime);

/// @brief Length of the tail to check after each envelop
static const unsigned int kTail(256);

/// @brief Generates an envelop, check for its range (must be >= 0)
TEST(Modulators, AdsdRange) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random parameters
    const unsigned int kAttack(kTimeDistribution(kRandomGenerator));
    const unsigned int kDecay(kTimeDistribution(kRandomGenerator));
    const unsigned int kSustain(kTimeDistribution(kRandomGenerator));
    const float kSustainLevel(kNormPosDistribution(kRandomGenerator));

    Adsd generator;
    generator.SetParameters(kAttack, kDecay, kDecay, kSustainLevel);

    const float kEpsilon(1e-3f);

    generator.TriggerOn();
    unsigned int i(0);
    // Attack / Decay / Sustain
    while (i <= kAttack + kDecay + kSustain) {
      const float sample(generator());
      EXPECT_LE(0.0f - kEpsilon, sample);
      EXPECT_GE(1.0f + kEpsilon, sample);
      i += 1;
    }
    // Release + a little bit after that
    generator.TriggerOff();
    while (i <= kAttack + kDecay + kSustain + kDecay + kTail) {
      const float sample(generator());
      EXPECT_LE(0.0f - kEpsilon, sample);
      EXPECT_GE(1.0f + kEpsilon, sample);
      i += 1;
    }
  }  // iterations?
}

/// @brief Used in the following test.
/// TODO(gm): get rid of this by using a more robust system (std::functional)
struct AdsdFunctor {
  explicit AdsdFunctor(Adsd* adsd)
    : adsd_(adsd),
      differentiator_() {
    // Nothing to do here
  }

  Sample operator()(void) {
    const Sample input(FillWithFloatGenerator(*adsd_));
    return differentiator_(input);
  }

 private:
  // No assignment operator for this class
  AdsdFunctor& operator=(const AdsdFunctor& right);

  Adsd* adsd_;
  Differentiator differentiator_;
};

/// @brief Generates an envelop, check for its proper timings:
/// - when in attack samples should be in a continuous upward slope
/// - when in decay samples should be in a continuous downward slope
/// - when in sustain samples should all be equal
/// - when in release samples should be in a continuous downward slope
TEST(Modulators, AdsdTimings) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random parameters
    const unsigned int kAttack(kTimeDistribution(kRandomGenerator));
    const unsigned int kDecay(kTimeDistribution(kRandomGenerator));
    const unsigned int kSustain(kTimeDistribution(kRandomGenerator));
    const float kSustainLevel(kNormPosDistribution(kRandomGenerator));

    Adsd generator;
    generator.SetParameters(kAttack, kDecay, kDecay, kSustainLevel);

    generator.TriggerOn();
    std::vector<unsigned int> zero_crossing_indexes;

    // TODO(gm): get rid of that
    AdsdFunctor adsd_functor(&generator);
    ZeroCrossing<AdsdFunctor> zero_crossing(adsd_functor);
    unsigned int kTriggerOnLength(kAttack + kDecay + kSustain);
    unsigned int kTotalLength(kTriggerOnLength + kDecay + kTail);
    // A tiny delay occurs due to differentiation and trigger unevenness
    unsigned int kEpsilon(6);
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
    EXPECT_NEAR(kAttack, zero_crossing_indexes[1], kEpsilon);
    EXPECT_NEAR(kAttack + kDecay, zero_crossing_indexes[2], kEpsilon);
    EXPECT_NEAR(kTriggerOnLength, zero_crossing_indexes[3], kEpsilon);
    EXPECT_NEAR(kTriggerOnLength + kDecay, zero_crossing_indexes[4], kEpsilon);
  }  // iterations?
}

/// @brief Check timings for long slopes
///
/// With the parameters used here, the slope is still supposed
/// to be > FLT_EPSILON
TEST(Modulators, AdsdLongTimings) {
  const unsigned int kLongTime(kMaxTime);
  const unsigned int kAttack(kLongTime);
  const unsigned int kDecay(kLongTime);
  const unsigned int kSustain(100);
  const float kSustainLevel(kNormPosDistribution(kRandomGenerator));

  Adsd generator;
  generator.SetParameters(kAttack, kDecay, kDecay, kSustainLevel);

  generator.TriggerOn();
  std::vector<unsigned int> zero_crossing_indexes;

  // TODO(gm): get rid of that
  AdsdFunctor adsd_functor(&generator);
  ZeroCrossing<AdsdFunctor> zero_crossing(adsd_functor);
  unsigned int kTriggerOnLength(kAttack + kDecay + kSustain);
  unsigned int kTotalLength(kTriggerOnLength + kDecay + kTail);
  // A tiny delay occurs due to differentiation and trigger unevenness
  unsigned int kEpsilon(4);
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
  EXPECT_NEAR(kAttack, zero_crossing_indexes[1], kEpsilon);
  EXPECT_NEAR(kAttack + kDecay, zero_crossing_indexes[2], kEpsilon);
  EXPECT_NEAR(kTriggerOnLength, zero_crossing_indexes[3], kEpsilon);
  EXPECT_NEAR(kTriggerOnLength + kDecay, zero_crossing_indexes[4], kEpsilon);
}

/// @brief Generates an envelop with one or both timing parameters null
TEST(Modulators, AdsdNullParameters) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random parameters
    // Each parameter has half a chance to be null
    const unsigned int kAttack(kBoolDistribution(kRandomGenerator)
                               ? kTimeDistribution(kRandomGenerator) : 0);
    const unsigned int kDecay(kBoolDistribution(kRandomGenerator)
                              ? kTimeDistribution(kRandomGenerator) : 0);
    const unsigned int kSustain(kTimeDistribution(kRandomGenerator));
    const float kSustainLevel(kNormPosDistribution(kRandomGenerator));

    Adsd generator;
    generator.SetParameters(kAttack, kDecay, kDecay, kSustainLevel);

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
    while (i <= kAttack + kDecay + kSustain) {
      const float sample(generator());
      // A (really tiny) epsilon is required here for imprecisions
      const float kNearEpsilon(1e-6f);
      EXPECT_NEAR(kSustainLevel, sample, kNearEpsilon);
      i += 1;
    }
    generator.TriggerOff();
    while (i < kAttack + kDecay + kSustain + kDecay + kTail) {
      const float sample(generator());
      EXPECT_LE(0.0f - kEpsilon, sample);
      EXPECT_GE(1.0f + kEpsilon, sample);
      i += 1;
    }
  }  // iterations?
}

/// @brief Generates a "click envelop" - with both timing parameters null
TEST(Modulators, AdsdClick) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const unsigned int kAttack(0);
    const unsigned int kDecay(0);
    const unsigned int kSustain(kTimeDistribution(kRandomGenerator));
    const float kSustainLevel(kNormPosDistribution(kRandomGenerator));

    Adsd generator;
    generator.SetParameters(kAttack, kDecay, kDecay, kSustainLevel);

    generator.TriggerOn();
    unsigned int i(1);
    // The first sample is always null!
    IGNORE(generator());
    while (i <= kSustain) {
      const float sample(generator());
      // A (really tiny) epsilon is required here for imprecisions
      const float kEpsilon(1e-6f);
      EXPECT_NEAR(kSustainLevel, sample, kEpsilon);
      i += 1;
    }
    generator.TriggerOff();
    // This sample left is due to the release
    IGNORE(generator());
    while (i <= kSustain + kTail) {
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
TEST(Modulators, AdsdOutRegularity) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random parameters
    const unsigned int kAttack(kTimeDistribution(kRandomGenerator));
    const unsigned int kDecay(kTimeDistribution(kRandomGenerator));
    const unsigned int kSustain(kTimeDistribution(kRandomGenerator));
    const float kSustainLevel(kNormPosDistribution(kRandomGenerator));

    Adsd generator;
    generator.SetParameters(kAttack, kDecay, kDecay, kSustainLevel);

    generator.TriggerOn();
    unsigned int i(1);
    // A very small epsilon is added for computation/casts imprecisions
    // TODO(gm): this might not be required if all floating point operations
    // were properly understood and managed.
    const double kMaxDelta(1.0 / std::min(kAttack, kDecay) + 1e-7);
    // Envelops should all begin at zero!
    double previous(static_cast<double>(generator()));
    // Checking the whole envelop since clicks may occur anywhere
    while (i < kAttack + kDecay + kSustain) {
      const double sample(static_cast<double>(generator()));
      const double diff(std::fabs(sample - previous));
      EXPECT_GE(kMaxDelta, diff);
      previous = sample;
      i += 1;
    }
    generator.TriggerOff();
    while (i < kAttack + kDecay + kSustain + kDecay + kTail) {
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
TEST(Modulators, AdsdPerf) {
  for (unsigned int iterations(0);
       iterations < kModulatorPerfIterations;
       ++iterations) {
    IGNORE(iterations);

    // Random parameters
    const unsigned int kAttack(kTimeDistribution(kRandomGenerator));
    const unsigned int kDecay(kTimeDistribution(kRandomGenerator));
    const unsigned int kSustain(kTimeDistribution(kRandomGenerator));
    const float kSustainLevel(kNormPosDistribution(kRandomGenerator));

    Adsd generator;
    generator.SetParameters(kAttack, kDecay, kDecay, kSustainLevel);

    generator.TriggerOn();

    unsigned int sample_idx(0);
    while (sample_idx < kAttack + kDecay + kSustain) {
      const float kCurrent(generator());
      sample_idx += 1;
      // No actual test!
      EXPECT_LE(-1.0f, kCurrent);
    }
    generator.TriggerOff();
    while (sample_idx < kModulatorDataPerfSetSize) {
      const float kCurrent(generator());
      sample_idx += 1;
      // No actual test!
      EXPECT_LE(-1.0f, kCurrent);
    }
  }  // iterations?
}
