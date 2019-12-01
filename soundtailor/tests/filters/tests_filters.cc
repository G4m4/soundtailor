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

#include <algorithm> // std::max

#include "soundtailor/tests/filters/tests_filters_fixture.h"

#include "soundtailor/src/filters/chamberlin.h"
#include "soundtailor/src/filters/chamberlin_oversampled.h"
#include "soundtailor/src/filters/firstorder_polezero.h"
#include "soundtailor/src/filters/firstorder_polefixedzero.h"
#include "soundtailor/src/filters/gain.h"
#include "soundtailor/src/filters/moog.h"
#include "soundtailor/src/filters/moog_lowaliasnonlinear.h"
#include "soundtailor/src/filters/moog_lowpassblock.h"
#include "soundtailor/src/filters/moog_oversampled.h"
#include "soundtailor/src/filters/oversampler.h"
#include "soundtailor/src/filters/secondorder_raw.h"

using soundtailor::filters::Chamberlin;
using soundtailor::filters::ChamberlinOversampled;
using soundtailor::filters::FirstOrderPoleZero;
using soundtailor::filters::FirstOrderPoleFixedZero;
using soundtailor::filters::Gain;
using soundtailor::filters::Moog;
using soundtailor::filters::MoogLowAliasNonLinear;
using soundtailor::filters::MoogLowPassBlock;
using soundtailor::filters::MoogOversampled;
using soundtailor::filters::Oversampler;
using soundtailor::filters::SecondOrderRaw;

/// @brief All tested filter types
typedef ::testing::Types<Chamberlin,
                         ChamberlinOversampled,
                         FirstOrderPoleZero,
                         FirstOrderPoleFixedZero,
                         Gain,
                         Moog,
                         MoogLowAliasNonLinear,
                         MoogLowPassBlock,
                         MoogOversampled,
                         Oversampler<SecondOrderRaw>,
                         SecondOrderRaw> FilterTypes;

/// @brief All filter types supporting passthrough
// @todo(gm) Chamberlin filter supports passthrough with a one-sample delay!
// @todo(gm) Passthrough issues with Oversampler, see #23
// @todo(gm) check FirstOrderPoleFixedZero, although not too much hope there...
typedef ::testing::Types<Chamberlin,
                         ChamberlinOversampled,
                         FirstOrderPoleZero,
                         Gain,
                         Moog,
                         MoogLowPassBlock,
                         Oversampler<SecondOrderRaw>,
                         SecondOrderRaw> PassthroughFilterTypes;

TYPED_TEST_SUITE(Filter, FilterTypes);
TYPED_TEST_SUITE(FilterData, FilterTypes);
TYPED_TEST_SUITE(FilterPassThrough, PassthroughFilterTypes);

/// @brief Filters a random signal, check for mean lower than the one
/// of the input signal (no DC offset introduced)
TYPED_TEST(Filter, ZeroOutputMean) {
  std::cerr << "Instance size : " << sizeof(TypeParam) << std::endl;
  for (unsigned int iterations(0); iterations < this->kTestIterations_; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(this->FilterFreqDistribution_(this->kRandomGenerator_));
    TypeParam filter;

    filter.SetParameters(kFrequency, this->kPassthroughResonance_);
    Sample expected_mean(VectorMath::Fill(0.0f));
    Sample actual_mean(VectorMath::Fill(0.0f));
    for (unsigned int i(0);
         i < this->kDataTestSetSize_;
         i += soundtailor::SampleSize) {
      const Sample input(VectorMath::Fill(this->kNormDistribution_(this->kRandomGenerator_)));
      const Sample filtered(filter(input));
      actual_mean = VectorMath::Add(actual_mean, filtered);
      expected_mean = VectorMath::Add(expected_mean, input);
    }
    const float kActual(std::abs(VectorMath::AddHorizontal(actual_mean)));
    const float kExpected(std::abs(VectorMath::AddHorizontal(expected_mean)));
    const float kEpsilon(3e-3f * this->kDataTestSetSize_);

    EXPECT_GT(kExpected + kEpsilon, kActual);
  }  // iterations?
}

/// @brief Check output range behaviour with max Q parameter without overshoot
TYPED_TEST(Filter, Range) {
  TypeParam filter;

  filter.SetParameters(this->kPassthroughFrequency_, this->kPassthroughResonance_);

  // Very high Epsilon due to this filter implementation
  const float kEpsilon(1e-1f);
  for (unsigned int i(0); i < this->kDataTestSetSize_; i += soundtailor::SampleSize) {
    const Sample input(VectorMath::Fill(this->kNormDistribution_(this->kRandomGenerator_)));
    const Sample filtered(VectorMath::MulConst(this->kInverseFilterGain_, filter(input)));
    EXPECT_TRUE(VectorMath::GreaterEqual(1.0f, VectorMath::Add(filtered, VectorMath::Fill(-kEpsilon))));
    EXPECT_TRUE(VectorMath::LessEqual(-1.0f, VectorMath::Add(filtered, VectorMath::Fill(kEpsilon))));
  }
}

/// @brief Check that both per-sample and per-block generation methods
/// yield an identical result
TYPED_TEST(FilterData, Process) {
  // Random normalized frequency
  const float kFrequency(this->FilterFreqDistribution_(this->kRandomGenerator_));

  TypeParam filter_perblock;
  TypeParam filter_persample;
  filter_perblock.SetParameters(kFrequency, this->kPassthroughResonance_);
  filter_persample.SetParameters(kFrequency, this->kPassthroughResonance_);

  soundtailor::ProcessBlock(
      &this->input_data_[0],
      &this->output_data_[0],
      this->output_data_.size(),
      filter_perblock);
  for (unsigned int i(0); i < this->kDataTestSetSize_; i += soundtailor::SampleSize) {
    const Sample kInput(VectorMath::Fill(&this->input_data_[i]));
    const Sample kReference(VectorMath::Fill(&this->output_data_[i]));
    const Sample kGenerated((filter_persample(kInput)));
    EXPECT_TRUE(VectorMath::Equal(kReference, kGenerated));
  }
}

/// @brief Filters random data (performance test)
TYPED_TEST(Filter, Perf) {
  for (unsigned int iterations(0); iterations < this->kPerfIterations_; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(this->FilterFreqDistribution_(this->kRandomGenerator_));
    TypeParam filter;
    filter.SetParameters(kFrequency, this->kPassthroughResonance_);

    unsigned int sample_idx(0);
    while (sample_idx < this->kDataTestSetSize_) {
      const Sample kCurrent(VectorMath::Fill(this->kNormDistribution_(this->kRandomGenerator_)));
      // No actual test!
      EXPECT_TRUE(VectorMath::LessEqual(-2.0f, filter(kCurrent)));
      sample_idx += soundtailor::SampleSize;
    }
  }
}

/// @brief Filters random data (block performance tests)
TYPED_TEST(FilterData, BlockPerf) {
  for (unsigned int iterations(0); iterations < this->kPerfIterations_; ++iterations) {
    IGNORE(iterations);
    const float kFrequency(this->FilterFreqDistribution_(this->kRandomGenerator_));
    TypeParam filter;
    filter.SetParameters(kFrequency, this->kPassthroughResonance_);

    soundtailor::ProcessBlock(
        &this->input_data_[0],
        &this->output_data_[0],
        this->output_data_.size(),
        filter);
    unsigned int sample_idx(0);
    while (sample_idx < this->kDataTestSetSize_) {
      const Sample kCurrent(VectorMath::Fill(&this->output_data_[sample_idx]));
      sample_idx += soundtailor::SampleSize;
      // No actual test!
      EXPECT_TRUE(VectorMath::LessEqual(-2.0f, kCurrent));
    }
  }
}

/// @brief Filters a random signal with max frequency cutoff and default Q
/// Check for minimal output/input error
TYPED_TEST(FilterPassThrough, Passthrough) {
  TypeParam filter;

  filter.SetParameters(this->kPassthroughFrequency_, this->kPassthroughResonance_);

  soundtailor::ProcessBlock(
    &this->input_data_[0],
    &this->output_data_[0],
    this->output_data_.size(),
    filter);
  float sum(0.0f);
  unsigned int input_idx(0);
  unsigned int output_idx(this->kDelay_);
  while (std::max(input_idx, output_idx) < this->kDataTestSetSize_) {
    // scalar codepath as we don't want to deal with alignment
    sum += this->kInverseFilterGain_ * this->input_data_[input_idx] - this->output_data_[output_idx];
    input_idx += 1;
    output_idx += 1;
  }
  const float kExpected(0.0f);
  const float kActual(sum);
  const float kEpsilon(2e-6f * this->kDataTestSetSize_);

  EXPECT_NEAR(kExpected, kActual, kEpsilon);
}
