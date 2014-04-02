/// @file tests_chamberlin.cc
/// @brief SoundTailor Chamberlin filters tests
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

#include "soundtailor/src/filters/chamberlin.h"

// Using declarations for tested filter
using soundtailor::filters::Chamberlin;

/// @brief Frequency parameter to be set in order to have a near-passthrough
static const float kPassthroughFrequency(1.0f);
/// @brief Resonance parameter to be set in order to have a near-passthrough
static const float kPassthroughResonance(1.0f);

/// @brief Filters a random signal, check for mean close to the one
/// of the input signal (no DC offset introduced)
TEST(Filters, ChamberlinOutputMean) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(kFreqDistribution(kRandomGenerator));
    Chamberlin filter;

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

    const float kActual(AddHorizontal(actual_mean));
    const float kExpected(AddHorizontal(expected_mean));
    const float kEpsilon(1e-3f * kDataTestSetSize);

    EXPECT_NEAR(kExpected, kActual, kEpsilon);
  }  // iterations?
}

/// @brief Filters a random signal with max frequency cutoff and default Q
/// Check for minimal output/input error
TEST(Filters, ChamberlinPassthrough) {
  std::vector<float> data_in(kDataTestSetSize);
  std::vector<float> data_out(kDataTestSetSize);
  std::generate(data_in.begin(),
                data_in.end(),
                std::bind(kNormDistribution, kRandomGenerator));
  Chamberlin filter;

  filter.SetParameters(kPassthroughFrequency, kPassthroughResonance);

  for (unsigned int i(0); i < kDataTestSetSize; i += soundtailor::SampleSize) {
    const Sample input(Fill(&data_in[i]));
    const Sample filtered(filter(input));
    Store(&data_out[i], filtered);
  }
  float diff_mean(0.0f);
  // There is a 1-sample delay!
  for (unsigned int i(0);
       i < kDataTestSetSize - 1;
       i += soundtailor::SampleSize) {
    diff_mean += data_in[i] - data_out[i + 1];
  }
  const float kExpected(0.0f);
  const float kActual(diff_mean);
  const float kEpsilon(1e-7f * kDataTestSetSize);

  EXPECT_NEAR(kExpected, kActual, kEpsilon);
}

/// @brief Filters random data (performance test)
TEST(Filters, ChamberlinPerf) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(kFreqDistribution(kRandomGenerator));
    Chamberlin filter;
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
