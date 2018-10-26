/// @file generators_common.h
/// @brief SoundTailor common stuff for all generators - declarations
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

#ifndef SOUNDTAILOR_SRC_GENERATORS_GENERATORS_COMMON_H_
#define SOUNDTAILOR_SRC_GENERATORS_GENERATORS_COMMON_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/maths.h"

namespace soundtailor {
namespace generators {

/// @brief Basic sawtooth signal generator
/// Generates a lot of aliasing, not to be used straight to audio
class PhaseAccumulator {
 public:
  explicit PhaseAccumulator(const float phase = 0.0f);
  Sample operator()(void);
  void SetPhase(const float phase);
  void SetFrequency(const float frequency);
  float ProcessParameters(void);

 private:
  Sample phase_;  ///< Instantaneous phase of the generator
  Sample increment_;  ///< Increment to be accumulated at each iteration
};

/// @brief Basic differentiator
/// implementing a simple 1st-order differentiator, unitary gain
class Differentiator {
 public:
  explicit Differentiator(const float last = 0.0f);
  Sample operator()(SampleRead sample);
  float ProcessParameters(float sample);

 private:
  float last_;  ///< Last synthesized sample value
};

}  // namespace generators
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_GENERATORS_GENERATORS_COMMON_H_
