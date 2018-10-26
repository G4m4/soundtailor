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
      : kTestIterations_( 16 ),

      // Smaller performance test sets in debug
#if (_SOUNDTAILOR_BUILD_CONFIGURATION_DEBUG)
    kPerfIterations_( 1 ),
#else  // (_SOUNDTAILOR_BUILD_CONFIGURATION_DEBUG)
    kPerfIterations_( 128 ),
#endif  // (_SOUNDTAILOR_BUILD_CONFIGURATION_DEBUG)

    kSamplingRate_(96000.0f),
    kMinTime_(0),
    kMaxTime_(static_cast<unsigned int>(kSamplingRate_)),
    kModulatorDataPerfSetSize_(kMaxTime_ * 4),
    kTail_(256),
    kRandomGenerator_(),
    kTimeDistribution_(kMinTime_, kMaxTime_),
    // Time values are multiple of 4
    kAttack_(GetMultipleOf4(kTimeDistribution_(kRandomGenerator_))),
    kDecay_(GetMultipleOf4(kTimeDistribution_(kRandomGenerator_))),
    kSustain_(GetMultipleOf4(kTimeDistribution_(kRandomGenerator_))),
    kSustainLevel_(kNormPosDistribution(kRandomGenerator_))
  {
    // Nothing to be done here for now
  }

  virtual ~Modulator() {
    // Nothing to be done here for now
  }

  const unsigned int kTestIterations_;
  const unsigned int kPerfIterations_;
  const float kSamplingRate_;
  /// @brief Arbitrary lowest allowed duration
  const unsigned int kMinTime_;
  /// @brief Arbitrary highest allowed duration
  const unsigned int kMaxTime_;
  const unsigned int kModulatorDataPerfSetSize_;
  /// @brief Length of the tail to check after each envelop
  const unsigned int kTail_;
  // @todo(gm) set the seed for deterministic tests across platforms
  std::default_random_engine kRandomGenerator_;
  /// @brief Time parameters random generator
  std::uniform_int_distribution<unsigned int> kTimeDistribution_;
  // Random parameters
  const unsigned int kAttack_;
  const unsigned int kDecay_;
  const unsigned int kSustain_;
  const float kSustainLevel_;

  /// @brief Used in the timing test.
  /// TODO(gm): get rid of this by using a more robust system (std::functional)
  struct AdsdFunctor {
    explicit AdsdFunctor(ModulatorType* modulators)
      : modulators_(modulators),
        differentiator_() {
      // Nothing to do here
    }

    Sample operator()(void) {
      const Sample input((*modulators_)());
      return differentiator_(input);
    }

   private:
    // No assignment operator for this class
    AdsdFunctor& operator=(const AdsdFunctor& right);

    ModulatorType* modulators_;
    soundtailor::generators::Differentiator differentiator_;
  };
};

/// @brief Base tests fixture data
template <typename ModulatorType>
class ModulatorData : public Modulator<ModulatorType> {
protected:

  ModulatorData()
    : output_data_(this->kModulatorDataPerfSetSize_) {
    // Nothing to be done here for now
  }

  virtual ~ModulatorData() {
    // Nothing to be done here for now
  }

  std::vector<float> output_data_;
};

#endif  // SOUNDTAILOR_TESTS_MODULATORS_TESTS_MODULATORS_FIXTURES_H_
