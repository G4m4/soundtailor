/// @file generators_common.cc
/// @brief SoundTailor common stuff for all generators - implementation
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

#include "soundtailor/src/generators/generators_common.h"
#include "soundtailor/src/maths.h"

namespace soundtailor {
namespace generators {

// PhaseAccumulator

PhaseAccumulator::PhaseAccumulator(const float phase)
    : Generator_Base(phase),
      phase_(Fill(phase)),
      increment_(Fill(0.0f)) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
}

Sample PhaseAccumulator::operator()(void) {
  const Sample out(phase_);
  phase_ = IncrementAndWrap(phase_, increment_);
  return out;
}

void PhaseAccumulator::SetPhase(const float phase) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
  // If we are not sure, we can use the following:
  // phase_ = Wrap(phase);
  phase_ = FillIncremental(phase, GetByIndex<0>(MulConst(0.25f, increment_)));
}

void PhaseAccumulator::SetFrequency(const float frequency) {
  SOUNDTAILOR_ASSERT(frequency >= 0.0f);
  SOUNDTAILOR_ASSERT(frequency <= 0.5f);

  const float base_increment(2.0f * frequency);
  increment_ = FillOnLength(base_increment);
  phase_ = FillIncremental(GetByIndex<0>(phase_), base_increment);
}

float PhaseAccumulator::ProcessParameters(void) {
  const float out(GetByIndex<0>(phase_));
  phase_ = IncrementAndWrap(phase_, Normalize(increment_));
  return out;
}

Differentiator::Differentiator(const float last)
    : last_(last) {
  // Nothing to do here
}

Sample Differentiator::operator()(SampleRead sample) {
  const float before_diff(GetLast(sample));
  const Sample prev(RotateOnRight(sample,
                                  last_));
  const Sample after_diff(Sub(sample, prev));
  last_ = before_diff;
  return after_diff;
}

float Differentiator::ProcessParameters(float sample) {
  const float before_diff(sample);
  const float prev(last_);
  const float after_diff(sample - prev);
  last_ = before_diff;
  return after_diff;
}

}  // namespace generators
}  // namespace soundtailor
