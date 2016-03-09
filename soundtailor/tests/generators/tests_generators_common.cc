/// @file tests_generators_common.cc
/// @brief SoundTailor common generators tests
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

// Using declarations for tested generator
using soundtailor::generators::PhaseAccumulator;
using soundtailor::generators::Differentiator;

const unsigned int kDataTestSetSize(32768);
const float kSamplingRate(96000.0f);
/// @brief Arbitrary lowest allowed fundamental
const float kMinFundamentalNorm(10.0f / kSamplingRate);
/// @brief Arbitrary highest allowed fundamental
// TODO(gm): make this higher
const float kMaxFundamentalNorm(2000.0f / kSamplingRate);

/// @brief Differentiate a constant, check for null derivative
TEST(GeneratorsCommon, DifferentiatedConstant) {
  std::default_random_engine kRandomGenerator;
  // The input is a random value in [-1.0f ; 1.0f]
  const Sample input(VectorMath::Fill(kNormDistribution(kRandomGenerator)));
  Differentiator differentiator;
  // Not checking the first value!
  differentiator(input);
  for (unsigned int i(soundtailor::SampleSize);
       i < kDataTestSetSize;
       i += soundtailor::SampleSize) {
    EXPECT_TRUE(VectorMath::Equal(0.0f, differentiator(input)));
  }
}

/// @brief Generates a triangle, check for its differentiated output:
/// it is supposed to be almost null everywhere except at discontinuities
TEST(GeneratorsCommon, DifferentiatedSawtooth) {
  std::default_random_engine kRandomGenerator;
  std::uniform_real_distribution<float> kFreqDistribution(kMinFundamentalNorm,
                                                          kMaxFundamentalNorm);
  const float kFrequency(kFreqDistribution(kRandomGenerator));
  PhaseAccumulator generator;
  generator.SetFrequency(kFrequency);

  // This is the sawtooth period e.g. each time the discontinuity occurs
  const int kPeriod(static_cast<int>(std::floor(kFrequency * kSamplingRate)));
  // The sawtooth is not perfect:
  // there may be a small DC offset for its derivative
  const float kThreshold(0.15f);

  Differentiator differentiator;
  for (unsigned int i(0); i < kDataTestSetSize; i += soundtailor::SampleSize) {
    const Sample input(generator());
    const Sample diff(differentiator(input));
    if (i % kPeriod != 0) {
      EXPECT_TRUE(VectorMath::GreaterThan(kThreshold, diff));
    }
  }
}

/// @brief Differentiate random values (performance test)
TEST(GeneratorsCommon, DifferentiatorPerf) {
  std::default_random_engine kRandomGenerator;

  // Smaller performance test sets in debug
#if (_BUILD_CONFIGURATION_DEBUG)
  const unsigned int kFilterDataPerfSetSize(16 * 1024);
#else  // (_BUILD_CONFIGURATION_DEBUG)
  const unsigned int kFilterDataPerfSetSize(16 * 1024 * 256);
#endif  // (_BUILD_CONFIGURATION_DEBUG)

  Differentiator differentiator;
  for (unsigned int i(0);
       i < kFilterDataPerfSetSize;
       i += soundtailor::SampleSize) {
    const Sample input(VectorMath::Fill(kNormDistribution(kRandomGenerator)));
    const Sample diff(differentiator(input));
    // No actual test!
    EXPECT_TRUE(VectorMath::LessThan(-2.0f, diff));
  }
}
