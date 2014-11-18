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

#include "soundtailor/tests/tests.h"

#include <random>

/// @brief Base tests fixture for all generators
template <typename GeneratorType>
class Generator : public ::testing::Test {
 protected:

  Generator()
      : kIterations(16),
    kSignalDataPeriodsCount(8.0f),
    kDataTestSetSize(32768),

// Smaller performance test sets in debug
#if (_BUILD_CONFIGURATION_DEBUG)
    kGeneratorDataPerfSetSize(16 * 1024),
#else  // (_BUILD_CONFIGURATION_DEBUG)
    kGeneratorDataPerfSetSize(16 * 1024 * 256),
#endif  // (_BUILD_CONFIGURATION_DEBUG)

    kSamplingRate(96000.0f),
    kMinFundamentalNorm(10.0f / kSamplingRate),
    kMaxFundamentalNorm(2000.0f / kSamplingRate),
    kMinKeyNote(0),
    kMaxKeyNote(93),
    kRandomGenerator(),
    kFreqDistribution(kMinFundamentalNorm, kMaxFundamentalNorm)
  {
    // Nothing to be done here for now
  }

  virtual ~Generator() {
    // Nothing to be done here for now
  }

  const unsigned int kIterations;
  const float kSignalDataPeriodsCount;
  const unsigned int kDataTestSetSize;
  const unsigned int kGeneratorDataPerfSetSize;
  /// @brief Base sampling rate unless indicated otherwise
  const float kSamplingRate;
  /// @brief Arbitrary lowest allowed fundamental
  const float kMinFundamentalNorm;
  /// @brief Arbitrary highest allowed fundamental
  // TODO(gm): make this higher
  const float kMaxFundamentalNorm;
  /// @brief Arbitrary lowest allowed key note (= C0)
  const unsigned int kMinKeyNote;
  /// @brief Arbitrary highest allowed key note (= A6)
  // TODO(gm): make this higher
  const unsigned int kMaxKeyNote;
  // @todo(gm) set the seed for deterministic tests across platforms
  std::default_random_engine kRandomGenerator;
  /// @brief Uniform distribution of normalized frequencies
  /// in ] 0.0f ; kMaxFundamentalNorm [
  std::uniform_real_distribution<float> kFreqDistribution;
};

#endif  // SOUNDTAILOR_TESTS_GENERATORS_TESTS_GENERATORS_FIXTURES_H_
