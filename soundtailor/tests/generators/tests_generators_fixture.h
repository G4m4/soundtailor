/// @file tests_generators_fixture.cc
/// @brief SoundTailor generators tests fixture
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

#ifndef SOUNDTAILOR_TESTS_GENERATORS_TESTS_GENERATORS_FIXTURES_H_
#define SOUNDTAILOR_TESTS_GENERATORS_TESTS_GENERATORS_FIXTURES_H_

#include <cmath>

// std::generate
#include <algorithm>
// std::bind
#include <functional>
#include <random>

#include "soundtailor/tests/tests.h"

/// @brief Base tests fixture for all generators
template <typename GeneratorType>
class Generator : public ::testing::Test {
 protected:

  Generator()
      :
      kTestIterations_( 16 ),

      // Smaller performance test sets in debug
#if (_BUILD_CONFIGURATION_DEBUG)
      kPerfIterations_( 1 ),
#else  // (_BUILD_CONFIGURATION_DEBUG)
      kPerfIterations_( 256 * 2 ),
#endif  // (_BUILD_CONFIGURATION_DEBUG)

    kSignalDataPeriodsCount_(8.0f),
    kDataTestSetSize_(32768),
    kSamplingRate_(96000.0f),
    kMinFundamentalNorm_(10.0f / kSamplingRate_),
    kMaxFundamentalNorm_(2000.0f / kSamplingRate_),
    kMinKeyNote_(0),
    kMaxKeyNote_(93),
    kRandomGenerator_(),
    kFreqDistribution_(kMinFundamentalNorm_, kMaxFundamentalNorm_)
  {
    // Nothing to be done here for now
  }

  virtual ~Generator() {
    // Nothing to be done here for now
  }

  const unsigned int kTestIterations_;
  const unsigned int kPerfIterations_;
  const float kSignalDataPeriodsCount_;
  const unsigned int kDataTestSetSize_;
  /// @brief Base sampling rate unless indicated otherwise
  const float kSamplingRate_;
  /// @brief Arbitrary lowest allowed fundamental
  const float kMinFundamentalNorm_;
  /// @brief Arbitrary highest allowed fundamental
  // TODO(gm): make this higher
  const float kMaxFundamentalNorm_;
  /// @brief Arbitrary lowest allowed key note (= C0)
  const unsigned int kMinKeyNote_;
  /// @brief Arbitrary highest allowed key note (= A6)
  // TODO(gm): make this higher
  const unsigned int kMaxKeyNote_;
  // @todo(gm) set the seed for deterministic tests across platforms
  std::default_random_engine kRandomGenerator_;
  /// @brief Uniform distribution of normalized frequencies
  /// in ] 0.0f ; kMaxFundamentalNorm_ [
  std::uniform_real_distribution<float> kFreqDistribution_;
};

/// @brief Base tests fixture data
template <typename GeneratorType>
class GeneratorData : public Generator<GeneratorType> {
 protected:

  GeneratorData()
      : output_data_(this->kDataTestSetSize_)
  {
    // Nothing to be done here for now
  }

  virtual ~GeneratorData() {
    // Nothing to be done here for now
  }

  std::vector<float> output_data_;
};

/// @brief Compute the mean value of a signal generator for the given length
///
/// @param[in]    generator      Generator to compute value from
/// @param[in]    length         Sample length
///
/// @return the generator mean for such length
template <typename TypeGenerator>
float ComputeMean(TypeGenerator& generator, const unsigned int length) {
  Sample sum(VectorMath::Fill(0.0f));
  unsigned int sample_idx(0);
  while (sample_idx < length) {
    const Sample sample(generator());
    sum = VectorMath::Add(sum, sample);
    sample_idx += soundtailor::SampleSize;
  }
  return VectorMath::AddHorizontal(sum) / static_cast<float>(length);
}

/// @brief Compute the mean power of a signal generator for the given length
///
/// @param[in]    generator      Generator to compute value from
/// @param[in]    length         Sample length
///
/// @return the generator mean for such length
template <typename TypeGenerator>
float ComputePower(TypeGenerator& generator, const unsigned int length) {
  Sample power(VectorMath::Fill(0.0f));
  unsigned int sample_idx(0);
  while (sample_idx < length) {
    const Sample sample(generator());
    const Sample squared(VectorMath::Mul(sample, sample));
    power = VectorMath::Add(power, squared);
    sample_idx += soundtailor::SampleSize;
  }
  return VectorMath::AddHorizontal(power) / static_cast<float>(length);
}

/// @brief Compute zero crossings of a signal generator for the given length
///
/// @param[in]  generator   Generator to compute value from
/// @param[in]  length    Sample length
/// @param[in]  initial_sgn   Initial generator sign, useful for generators
///                           beginning at 0 and decreasing (Triangle DPW...)
///
/// @return zero crossings occurence for such length
template <typename TypeGenerator>
int ComputeZeroCrossing(TypeGenerator& generator,
                        const unsigned int length,
                        const float initial_sgn = 1.0f) {
  ZeroCrossing<TypeGenerator> zero_crossing(generator, initial_sgn);
  int out(0);
  unsigned int zero_crossing_idx(zero_crossing.GetNextZeroCrossing(length));
  while (zero_crossing_idx < length) {
    out += 1;
    zero_crossing_idx = zero_crossing.GetNextZeroCrossing(length);
  }
  return out;
}

/// @brief Compute the frequency of a given piano key (A4 = 440Hz)
static inline float NoteToFrequency(const unsigned int key_number) {
  const float exponent((static_cast<float>(key_number) - 69.0f) / 12.0f);
  return std::pow(2.0f, exponent) * 440.0f;
}

/// @brief Helper structure for checking a signal continuity
struct IsContinuous {
  /// @brief Default constructor
  ///
  /// @param[in]  threshold   Max difference between two consecutive samples
  /// @param[in]  previous   First sample initialization
  IsContinuous(const float threshold, const float previous)
      : threshold_(threshold),
        previous_(previous) {
    SOUNDTAILOR_ASSERT(threshold >= 0.0f);
  }

  /// @brief Check next sample continuity
  ///
  /// @param[in]  input   Sample to be tested
  bool operator()(SampleRead input) {
    const float before_diff(VectorMath::GetLast(input));
    const Sample prev(VectorMath::RotateOnRight(input,
                                    previous_));
    const Sample after_diff(VectorMath::Sub(input, prev));
    previous_ = before_diff;
    if (VectorMath::LessThan(threshold_, VectorMath::Abs(after_diff))) {
      return false;
    }
    return true;
  }

  float threshold_;
  float previous_;
};

/// @brief Compute how many samples are required in order to have exactly
/// the given number of periods for the given signal frequency
///
/// @param[in]  frequency   Signal frequency (normalised)
/// @param[in]  sampling rate   In Hertz
/// @param[in]  period_count    Expected number of period (may be non-integer)
static inline unsigned int ComputeDataLength(const float frequency,
                                             const float period_count) {
  return static_cast<unsigned int>(std::floor(period_count / frequency));
}

#endif  // SOUNDTAILOR_TESTS_GENERATORS_TESTS_GENERATORS_FIXTURES_H_
