/// @file triangle_dpw.cc
/// @brief Triangle signal generator using DPW algorithm - implementation
/// @author gm
/// @copyright gm 2016
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

// std::fabs
#include <cmath>

#include "soundtailor/src/generators/triangle_dpw.h"
#include "soundtailor/src/maths.h"

namespace soundtailor {
namespace generators {

TriangleDPW::TriangleDPW(const float phase)
    : sawtooth_gen_(),
      differentiator_(),
      normalization_factor_(0.0f) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
  SetPhase(phase);
  ProcessParameters();
}

Sample TriangleDPW::operator()(void) {
  // Raw sawtooth signal
  Sample current(sawtooth_gen_());
  const Sample current_abs(VectorMath::Abs(current));
  // Parabolization
  const Sample squared(VectorMath::Mul(current, current_abs));
  const Sample minus(VectorMath::Sub(current, squared));
  // Differentiation & Normalization
  const Sample diff(differentiator_(minus));
  return VectorMath::MulConst(normalization_factor_, diff);
}

void TriangleDPW::SetPhase(const float phase) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
  // there might be a phase derivative issue (e.g. an increasing phase
  // changed for a decreasing one) but the signal is still continuous
  const float actual_phase = phase * -0.5f + 0.5f;
  sawtooth_gen_.SetPhase(actual_phase);
}

void TriangleDPW::SetFrequency(const float frequency) {
  SOUNDTAILOR_ASSERT(frequency >= 0.0f);
  SOUNDTAILOR_ASSERT(frequency <= 0.5f);

  sawtooth_gen_.SetFrequency(frequency);
  normalization_factor_ = 1.0f / (2.0f * frequency);
}

float TriangleDPW::ProcessParameters(void) {
  const float current(sawtooth_gen_.ProcessParameters());
  const float current_abs(std::fabs(current));
  const float squared(current * current_abs);
  const float minus(current - squared);
  return normalization_factor_ * differentiator_.ProcessParameters(minus);
}

}  // namespace generators
}  // namespace soundtailor
