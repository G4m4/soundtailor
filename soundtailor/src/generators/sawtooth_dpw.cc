/// @file sawtooth_dpw.cc
/// @brief Sawtooth signal generator using DPW algorithm - implementation
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

#include "soundtailor/src/generators/sawtooth_dpw.h"
#include "soundtailor/src/maths.h"

namespace soundtailor {
namespace generators {

SawtoothDPW::SawtoothDPW(const float phase)
    : sawtooth_gen_(),
      differentiator_(),
      normalization_factor_(0.0f) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
  SetPhase(phase);
  ProcessParameters();
}

Sample SawtoothDPW::operator()(void) {
  // Raw sawtooth signal
  const Sample current(sawtooth_gen_());
  // Parabolization
  const Sample squared(VectorMath::Mul(current, current));
  // Differentiation & Normalization
  const Sample diff(differentiator_(squared));
  return VectorMath::MulConst(normalization_factor_, diff);
}

void SawtoothDPW::SetPhase(const float phase) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
  sawtooth_gen_.SetPhase(phase);
}

void SawtoothDPW::SetFrequency(const float frequency) {
  SOUNDTAILOR_ASSERT(frequency >= 0.0f);
  SOUNDTAILOR_ASSERT(frequency <= 0.5f);

  sawtooth_gen_.SetFrequency(frequency);
  normalization_factor_ = 1.0f / (4.0f * frequency);
}

float SawtoothDPW::ProcessParameters(void) {
  const float current(sawtooth_gen_.ProcessParameters());
  const float squared(current * current);
  return normalization_factor_ * differentiator_.ProcessParameters(squared);
}

}  // namespace generators
}  // namespace soundtailor
