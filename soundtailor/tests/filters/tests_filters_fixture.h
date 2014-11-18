/// @file tests_filters_fixture.cc
/// @brief SoundTailor filters tests fixture
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

#ifndef SOUNDTAILOR_TESTS_FILTERS_TESTS_FILTERS_FIXTURES_H_
#define SOUNDTAILOR_TESTS_FILTERS_TESTS_FILTERS_FIXTURES_H_

#include "soundtailor/tests/tests.h"

#include <random>

/// @brief Base tests fixture for all filters
template <typename FilterType>
class Filter : public ::testing::Test {
 protected:

  Filter()
      : kDataTestSetSize(32768),
    kIterations(16),

  // Smaller performance test sets in debug
#if (_BUILD_CONFIGURATION_DEBUG)
    kFilterDataPerfSetSize(16 * 1024),
#else  // (_BUILD_CONFIGURATION_DEBUG)
    kFilterDataPerfSetSize(16 * 1024 * 256),
#endif  // (_BUILD_CONFIGURATION_DEBUG)

    kRandomGenerator(),
    kNormDistribution(-1.0f, 1.0f),
    kPassthroughFrequency(FilterType::Meta().freq_passthrough),
    kPassthroughResonance(FilterType::Meta().res_passthrough),
    kDelay(FilterType::Meta().output_delay),
    kInverseFilterGain(1.0f / FilterType::Meta().output_gain),
    FilterFreqDistribution(FilterType::Meta().freq_min,
                           FilterType::Meta().freq_max)
  {
    // You can do set-up work for each test here.
  }

  virtual ~Filter() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  const unsigned int kDataTestSetSize;
  const unsigned int kIterations;
  const unsigned int kFilterDataPerfSetSize;
  // @todo(gm) set the seed for deterministic tests across platforms
  std::default_random_engine kRandomGenerator;
  std::uniform_real_distribution<float> kNormDistribution;

  /// @brief Frequency parameter to be set in order to have a near-passthrough
  const float kPassthroughFrequency;
  /// @brief Resonance parameter to be set in order to have a near-passthrough
  const float kPassthroughResonance;
  /// @brief Filter delay
  const unsigned int kDelay;
  /// @brief Inverse of the gain introduced by the filter
  const float kInverseFilterGain;
  /// @brief Random distribution for filter frequency, within its own bounds
  std::uniform_real_distribution<float> FilterFreqDistribution;
};

/// @brief Base tests fixture for all filters able to be passthrough
template <typename FilterType>
class FilterPassThrough : public Filter<FilterType> {
};

#endif  // SOUNDTAILOR_TESTS_FILTERS_TESTS_FILTERS_FIXTURES_H_
