/// @file tests_moog_oversampled.cc
/// @brief Oversampled Moog filter specific tests
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

#include "soundtailor/src/filters/moog_oversampled.h"

// Using declarations for tested filter
using soundtailor::filters::MoogOversampled;

/// @brief Frequency parameter to be set in order to have a near-passthrough
static const float kPassthroughFrequency(MoogOversampled::Meta().freq_passthrough);
/// @brief Resonance parameter to be set in order to have a near-passthrough
static const float kPassthroughResonance(MoogOversampled::Meta().res_passthrough);

/// @brief Random distribution for filter frequency, within its own bounds
std::uniform_real_distribution<float> MoogOversampledFilterFreqDistribution(
  MoogOversampled::Meta().freq_min,
  MoogOversampled::Meta().freq_max);

/// @brief Filters a random signal, check for mean lower than the one
/// of the input signal (no DC offset introduced)
TEST(Filters, MoogOversampledOutputMean) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(MoogOversampledFilterFreqDistribution(kRandomGenerator));
    MoogOversampled filter;

    filter.SetParameters(kFrequency, kPassthroughResonance);
    Sample expected_mean(Fill(0.0f));
    Sample actual_mean(Fill(0.0f));
    for (unsigned int i(0);
         i < kDataTestSetSize;
         i += soundtailor::SampleSize) {
      const Sample input(Fill(kNormDistribution(kRandomGenerator)));
      const Sample filtered(filter(input));
      actual_mean = Add(actual_mean, filtered);
      expected_mean = Add(expected_mean, input);
    }
    const float kActual(std::abs(AddHorizontal(actual_mean)));
    const float kExpected(std::abs(AddHorizontal(expected_mean)));
    const float kEpsilon(1e-3f * kDataTestSetSize);

    EXPECT_GT(kExpected + kEpsilon, kActual);
  }  // iterations?
}

/// @brief Check output range behaviour with max Q parameter without overshoot
TEST(Filters, MoogOversampledRange) {
  MoogOversampled filter;

  filter.SetParameters(kPassthroughFrequency, kPassthroughResonance);

  // Very high Epsilon due to this filter implementation
  const float kEpsilon(1e-1f);
  for (unsigned int i(0); i < kDataTestSetSize; i += soundtailor::SampleSize) {
    const Sample input(Fill(kNormDistribution(kRandomGenerator)));
    const Sample filtered(filter(input));
    EXPECT_TRUE(GreaterEqual(1.0f, Add(filtered, Fill(-kEpsilon))));
    EXPECT_TRUE(LessEqual(-1.0f, Add(filtered, Fill(kEpsilon))));
  }
}

/// @brief Filters random data (performance test)
TEST(Filters, MoogOversampledPerf) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(MoogOversampledFilterFreqDistribution(kRandomGenerator));
    MoogOversampled filter;
    filter.SetParameters(kFrequency, kPassthroughResonance);

    unsigned int sample_idx(0);
    while (sample_idx < kFilterDataPerfSetSize) {
      const Sample kCurrent(Fill(kNormDistribution(kRandomGenerator)));
      // No actual test!
      EXPECT_TRUE(LessEqual(-2.0f, filter(kCurrent)));
      sample_idx += soundtailor::SampleSize;
    }
  }
}
