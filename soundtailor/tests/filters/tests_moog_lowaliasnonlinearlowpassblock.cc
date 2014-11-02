/// @file tests_moog_lowaliasnonlinearlowpassblock.cc
/// @brief MoogLowAliasNonLinear low-alias nonlinear lowpass building block tests
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

#include "soundtailor/src/filters/moog_lowaliasnonlinear_lowpassblock.h"

// Using declarations for tested filter
using soundtailor::filters::MoogLowAliasNonLinearLowPassBlock;

/// @brief Frequency parameter to be set in order to have a near-passthrough
static const float kPassthroughFrequency(MoogLowAliasNonLinearLowPassBlock::Meta().freq_passthrough);
/// @brief Resonance parameter to be set in order to have a near-passthrough
static const float kPassthroughResonance(MoogLowAliasNonLinearLowPassBlock::Meta().res_passthrough);

/// @brief Random distribution for filter frequency, within its own bounds
std::uniform_real_distribution<float> MoogLowAliasNonLinearLowPassBlockFilterFreqDistribution(
  MoogLowAliasNonLinearLowPassBlock::Meta().freq_min,
  MoogLowAliasNonLinearLowPassBlock::Meta().freq_max);

/// @brief Filters a random signal, check for mean lower than the one
/// of the input signal (no DC offset introduced)
TEST(Filters, MoogLowAliasNonLinearLowPassBlockZeroOutputMean) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(MoogLowAliasNonLinearLowPassBlockFilterFreqDistribution(kRandomGenerator));
    MoogLowAliasNonLinearLowPassBlock filter;

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
    const float kEpsilon(3e-3f * kDataTestSetSize);

    EXPECT_GT(kExpected + kEpsilon, kActual);
  }  // iterations?
}

/// @brief Filters a random signal with max frequency cutoff and default Q
/// Check for minimal output/input error
TEST(Filters, MoogLowAliasNonLinearLowPassBlockPassthrough) {
  MoogLowAliasNonLinearLowPassBlock filter;

  filter.SetParameters(kPassthroughFrequency, kPassthroughResonance);

  Sample diff_mean(Fill(0.0f));
  for (unsigned int i(0); i < kDataTestSetSize; i += soundtailor::SampleSize) {
    const Sample input(Fill(kNormDistribution(kRandomGenerator)));
    // This filter introduce a fixed gain
    // @todo(gm) a generic way to handle this
    const Sample filtered(MulConst(1.0f / 1.3f, filter(input)));
    diff_mean = Add(diff_mean, Sub(filtered, input));
  }
  const float kExpected(0.0f);
  const float kActual(AddHorizontal(diff_mean));
  const float kEpsilon(1e-6f * kDataTestSetSize);

  EXPECT_NEAR(kExpected, kActual, kEpsilon);
}

/// @brief Check output range behaviour with max Q parameter without overshoot
TEST(Filters, MoogLowAliasNonLinearLowPassBlockRange) {
  MoogLowAliasNonLinearLowPassBlock filter;

  filter.SetParameters(kPassthroughFrequency, kPassthroughResonance);

  // Very high Epsilon due to this filter implementation
  const float kEpsilon(1e-1f);
  for (unsigned int i(0); i < kDataTestSetSize; i += soundtailor::SampleSize) {
    const Sample input(Fill(kNormDistribution(kRandomGenerator)));
    // This filter introduce a fixed gain
    // @todo(gm) a generic way to handle this
    const Sample filtered(MulConst(1.0f / 1.3f, filter(input)));
    EXPECT_TRUE(GreaterEqual(1.0f, Add(filtered, Fill(-kEpsilon))));
    EXPECT_TRUE(LessEqual(-1.0f, Add(filtered, Fill(kEpsilon))));
  }
}

/// @brief Filters random data (performance test)
TEST(Filters, MoogLowAliasNonLinearLowPassBlockPerf) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(MoogLowAliasNonLinearLowPassBlockFilterFreqDistribution(kRandomGenerator));
    MoogLowAliasNonLinearLowPassBlock filter;
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
