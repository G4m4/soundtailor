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
#if (_SOUNDTAILOR_BUILD_CONFIGURATION_DEBUG)
      kPerfIterations_( 1 ),
#else  // (_SOUNDTAILOR_BUILD_CONFIGURATION_DEBUG)
      kPerfIterations_( 256 * 2 ),
#endif  // (_SOUNDTAILOR_BUILD_CONFIGURATION_DEBUG)

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

/// @brief Tests fixture for generators able to output zero
// @todo(gm) get rid of that by using generators policies
template <typename GeneratorType>
class GeneratorWithZero : public Generator<GeneratorType> {
};

/// @brief Compute the frequency of a given piano key (A4 = 440Hz)
static inline float NoteToFrequency(const unsigned int key_number) {
  const float exponent((static_cast<float>(key_number) - 69.0f) / 12.0f);
  return std::pow(2.0f, exponent) * 440.0f;
}

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
