/// @file tests_filters.cc
/// @brief SoundTailor filters common tests
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

#include "soundtailor/tests/filters/tests_filters_fixture.h"

#include "soundtailor/src/filters/chamberlin.h"
#include "soundtailor/src/filters/chamberlin_oversampled.h"
#include "soundtailor/src/filters/firstorder_polezero.h"
#include "soundtailor/src/filters/moog.h"
#include "soundtailor/src/filters/moog_lowaliasnonlinear.h"
#include "soundtailor/src/filters/moog_lowaliasnonlinear_lowpassblock.h"
#include "soundtailor/src/filters/moog_lowpassblock.h"
#include "soundtailor/src/filters/moog_oversampled.h"
#include "soundtailor/src/filters/oversampler.h"
#include "soundtailor/src/filters/secondorder_raw.h"

using soundtailor::filters::Chamberlin;
using soundtailor::filters::ChamberlinOversampled;
using soundtailor::filters::FirstOrderPoleZero;
using soundtailor::filters::Moog;
using soundtailor::filters::MoogLowAliasNonLinear;
using soundtailor::filters::MoogLowAliasNonLinearLowPassBlock;
using soundtailor::filters::MoogLowPassBlock;
using soundtailor::filters::MoogOversampled;
using soundtailor::filters::Oversampler;
using soundtailor::filters::SecondOrderRaw;

/// @brief All tested filter types
typedef ::testing::Types<Chamberlin,
                         ChamberlinOversampled,
                         FirstOrderPoleZero,
                         Moog,
                         MoogLowAliasNonLinear,
                         MoogLowAliasNonLinearLowPassBlock,
                         MoogLowPassBlock,
                         MoogOversampled,
                         Oversampler<SecondOrderRaw>,
                         SecondOrderRaw> FilterTypes;

/// @brief All filter types supporting passthrough
// @todo(gm) Chamberlin filter supports passthrough with a one-sample delay!
// @todo(gm) Passthrough issues with Oversampler, see #23
typedef ::testing::Types<ChamberlinOversampled,
                         FirstOrderPoleZero,
                         Moog,
                         MoogLowAliasNonLinearLowPassBlock,
                         MoogLowPassBlock,
                         SecondOrderRaw> PassthroughFilterTypes;

TYPED_TEST_CASE(Filter, FilterTypes);
TYPED_TEST_CASE(FilterPassThrough, PassthroughFilterTypes);

/// @brief Filters a random signal, check for mean lower than the one
/// of the input signal (no DC offset introduced)
TYPED_TEST(Filter, ZeroOutputMean) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(this->FilterFreqDistribution(this->kRandomGenerator));
    TypeParam filter;

    filter.SetParameters(kFrequency, this->kPassthroughResonance);
    Sample expected_mean(Fill(0.0f));
    Sample actual_mean(Fill(0.0f));
    for (unsigned int i(0);
         i < kDataTestSetSize;
         i += soundtailor::SampleSize) {
      const Sample input(Fill(this->kNormDistribution(this->kRandomGenerator)));
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

/// @brief Check output range behaviour with max Q parameter without overshoot
TYPED_TEST(Filter, Range) {
  TypeParam filter;

  filter.SetParameters(this->kPassthroughFrequency, this->kPassthroughResonance);

  // Very high Epsilon due to this filter implementation
  const float kEpsilon(1e-1f);
  for (unsigned int i(0); i < this->kDataTestSetSize; i += soundtailor::SampleSize) {
    const Sample input(Fill(this->kNormDistribution(this->kRandomGenerator)));
    const Sample filtered(MulConst(this->kInverseFilterGain, filter(input)));
    EXPECT_TRUE(GreaterEqual(1.0f, Add(filtered, Fill(-kEpsilon))));
    EXPECT_TRUE(LessEqual(-1.0f, Add(filtered, Fill(kEpsilon))));
  }
}

/// @brief Filters random data (performance test)
TYPED_TEST(Filter, Perf) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(this->FilterFreqDistribution(this->kRandomGenerator));
    TypeParam filter;
    filter.SetParameters(kFrequency, this->kPassthroughResonance);

    unsigned int sample_idx(0);
    while (sample_idx < this->kFilterDataPerfSetSize) {
      const Sample kCurrent(Fill(this->kNormDistribution(this->kRandomGenerator)));
      // No actual test!
      EXPECT_TRUE(LessEqual(-2.0f, filter(kCurrent)));
      sample_idx += soundtailor::SampleSize;
    }
  }
}

/// @brief Filters a random signal with max frequency cutoff and default Q
/// Check for minimal output/input error
TYPED_TEST(FilterPassThrough, Passthrough) {
  TypeParam filter;

  filter.SetParameters(this->kPassthroughFrequency, this->kPassthroughResonance);

  Sample diff_mean(Fill(0.0f));
  for (unsigned int i(0);
       i < this->kDataTestSetSize;
       i += soundtailor::SampleSize) {
    const Sample input(Fill(this->kNormDistribution(this->kRandomGenerator)));
    const Sample filtered(MulConst(this->kInverseFilterGain, filter(input)));
    diff_mean = Add(diff_mean, Sub(filtered, input));
  }
  const float kExpected(0.0f);
  const float kActual(AddHorizontal(diff_mean));
  const float kEpsilon(2e-6f * this->kDataTestSetSize);

  EXPECT_NEAR(kExpected, kActual, kEpsilon);
}