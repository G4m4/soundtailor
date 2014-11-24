/// @file tests_modulators_fixture.cc
/// @brief SoundTailor modulators tests fixture
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

#ifndef SOUNDTAILOR_TESTS_MODULATORS_TESTS_MODULATORS_FIXTURES_H_
#define SOUNDTAILOR_TESTS_MODULATORS_TESTS_MODULATORS_FIXTURES_H_

#include "soundtailor/tests/tests.h"
#include "soundtailor/src/generators/generators_common.h"

// std::generate
#include <algorithm>
// std::bind
#include <functional>
#include <random>

/// @brief Base tests fixture for modulators
template <typename ModulatorType>
class Modulator : public ::testing::Test {
 protected:

  Modulator()
      : kTestIterations( 16 ),

      // Smaller performance test sets in debug
#if (_BUILD_CONFIGURATION_DEBUG)
    kPerfIterations( 1 ),
#else  // (_BUILD_CONFIGURATION_DEBUG)
    kPerfIterations( 128 ),
#endif  // (_BUILD_CONFIGURATION_DEBUG)

    kMinTime(0),
    kMaxTime(static_cast<unsigned int>(kSamplingRate)),
    kModulatorDataPerfSetSize(kMaxTime * 4),
    kTail(256),
    kRandomGenerator(),
    kTimeDistribution(kMinTime, kMaxTime),
    kAttack(kTimeDistribution(kRandomGenerator)),
    kDecay(kTimeDistribution(kRandomGenerator)),
    kSustain(kTimeDistribution(kRandomGenerator)),
    kSustainLevel(kNormPosDistribution(kRandomGenerator))
  {
    // Nothing to be done here for now
  }

  virtual ~Modulator() {
    // Nothing to be done here for now
  }

  const unsigned int kTestIterations;
  const unsigned int kPerfIterations;
  /// @brief Arbitrary lowest allowed duration
  const unsigned int kMinTime;
  /// @brief Arbitrary highest allowed duration
  const unsigned int kMaxTime;
  const unsigned int kModulatorDataPerfSetSize;
  /// @brief Length of the tail to check after each envelop
  const unsigned int kTail;
  // @todo(gm) set the seed for deterministic tests across platforms
  std::default_random_engine kRandomGenerator;
  /// @brief Time parameters random generator
  std::uniform_int_distribution<unsigned int> kTimeDistribution;
  // Random parameters
  const unsigned int kAttack;
  const unsigned int kDecay;
  const unsigned int kSustain;
  const float kSustainLevel;

  /// @brief Used in the timing test.
  /// TODO(gm): get rid of this by using a more robust system (std::functional)
  struct AdsdFunctor {
    explicit AdsdFunctor(ModulatorType* modulators)
      : modulators_(modulators),
        differentiator_() {
      // Nothing to do here
    }

    Sample operator()(void) {
      const Sample input(FillWithFloatGenerator(*modulators_));
      return differentiator_(input);
    }

   private:
    // No assignment operator for this class
    AdsdFunctor& operator=(const AdsdFunctor& right);

    ModulatorType* modulators_;
    soundtailor::generators::Differentiator differentiator_;
  };
};

#endif  // SOUNDTAILOR_TESTS_MODULATORS_TESTS_MODULATORS_FIXTURES_H_
