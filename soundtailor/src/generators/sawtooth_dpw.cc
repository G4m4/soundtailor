/// @file sawtooth_dpw.cc
/// @brief Sawtooth signal generator using DPW algorithm - implementation
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

#include "soundtailor/src/generators/sawtooth_dpw.h"
#include "soundtailor/src/maths.h"

namespace soundtailor {
namespace generators {

SawtoothDPW::SawtoothDPW(const float phase)
    : TriangleDPW(phase) {
  ASSERT(phase <= 1.0f);
  ASSERT(phase >= -1.0f);
}

Sample SawtoothDPW::operator()(void) {
  // Raw sawtooth signal
  const Sample current(sawtooth_gen_());
  // Parabolization
  const Sample squared(Mul(current, current));
  // Differentiation & Normalization
  const Sample diff(differentiator_(squared));
  return MulConst(normalization_factor_, diff);
}

float SawtoothDPW::ProcessScalar(void) {
  // Beware: not efficient, only to be used when a 1-sample delay is required
  const float current(sawtooth_gen_.ProcessScalar());
  const float squared(current * current);
  return normalization_factor_ * differentiator_.ProcessScalar(squared);
}

void SawtoothDPW::SetFrequency(const float frequency) {
  ASSERT(frequency >= 0.0f);
  ASSERT(frequency <= 0.5f);

  sawtooth_gen_.SetFrequency(frequency);
  normalization_factor_ = 1.0f / (4.0f * frequency);
}

}  // namespace generators
}  // namespace soundtailor
