/// @file tests_chamberlingoversampled.cc
/// @brief SoundTailor Oversampled Chamberlin filters tests
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

#include "soundtailor/src/filters/chamberlin_oversampled.h"

// Using declarations for tested filter
using soundtailor::filters::ChamberlinOversampled;

/// @brief Frequency parameter to be set in order to have a near-passthrough
static const float kPassthroughFrequency(ChamberlinOversampled::Meta().freq_passthrough);
/// @brief Resonance parameter to be set in order to have a near-passthrough
static const float kPassthroughResonance(ChamberlinOversampled::Meta().res_passthrough);
/// @brief Filter delay
static const unsigned int kDelay(ChamberlinOversampled::Meta().output_delay);

/// @brief Random distribution for filter frequency, within its own bounds
std::uniform_real_distribution<float> ChamberlinOversampledFilterFreqDistribution(
  ChamberlinOversampled::Meta().freq_min,
  ChamberlinOversampled::Meta().freq_max);

/// @brief Filters a random signal, check for mean lower than the one
/// of the input signal (no DC offset introduced)
TEST(Filters, ChamberlinOversampledOutputMean) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(ChamberlinOversampledFilterFreqDistribution(kRandomGenerator));
    ChamberlinOversampled filter;

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

/// @brief Filters a random signal with max frequency cutoff and default Q
/// Check for minimal output/input error
TEST(Filters, ChamberlinOversampledPassthrough) {
  std::vector<float> data_in(kDataTestSetSize);
  std::vector<float> data_out(kDataTestSetSize);
  std::generate(data_in.begin(),
                data_in.end(),
                std::bind(kNormDistribution, kRandomGenerator));
  ChamberlinOversampled filter;

  filter.SetParameters(kPassthroughFrequency, kPassthroughResonance);

  for (unsigned int i(0); i < kDataTestSetSize; i += soundtailor::SampleSize) {
    const Sample input(Fill(&data_in[i]));
    const Sample filtered(filter(input));
    Store(&data_out[i], filtered);
  }
  float diff_mean(0.0f);
  for (unsigned int i(0);
       i < kDataTestSetSize - kDelay;
       i += soundtailor::SampleSize) {
    diff_mean += data_in[i] - data_out[i + kDelay];
  }
  const float kExpected(0.0f);
  const float kActual(diff_mean);
  const float kEpsilon(1e-7f * kDataTestSetSize);

  EXPECT_NEAR(kExpected, kActual, kEpsilon);
}

/// @brief Filters random data (performance test)
TEST(Filters, ChamberlinOversampledPerf) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(ChamberlinOversampledFilterFreqDistribution(kRandomGenerator));
    ChamberlinOversampled filter;
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