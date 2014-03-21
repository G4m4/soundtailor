/// @file triangle_dpw.cc
/// @brief Triangle signal generator using DPW algorithm - implementation
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

#include "soundtailor/src/generators/triangle_dpw.h"
#include "soundtailor/src/maths.h"

namespace soundtailor {
namespace generators {

TriangleDPW::TriangleDPW(const float phase)
    : Generator_Base(phase),
      sawtooth_gen_(phase),
      differentiator_(),
      normalization_factor_(0.0f) {
  ASSERT(phase <= 1.0f);
  ASSERT(phase >= -1.0f);
}

Sample TriangleDPW::operator()(void) {
  // Raw sawtooth signal
  Sample current(sawtooth_gen_());
  const Sample current_abs(Abs(current));
  // Parabolization
  const Sample squared(Mul(current, current_abs));
  const Sample minus(Sub(current, squared));
  // Differentiation & Normalization
  const Sample diff(differentiator_(minus));
  return MulConst(normalization_factor_, diff);
}

void TriangleDPW::SetPhase(const float phase) {
  // Phase is supposed to be in [-1.0 ; 1.0], hence the assert
  ASSERT(phase <= 1.0f);
  ASSERT(phase >= -1.0f);
  // This is an arbitrary value below which samples are considered equal
  // TODO(gm): a clean definition for this
  const float kEpsilon(1e-6f);
  // TODO(gm): a clean, analytic resolution of this
  while (!IsAnyNear(Fill(phase), this->operator()(), kEpsilon));
}

void TriangleDPW::SetFrequency(const float frequency) {
  ASSERT(frequency >= 0.0f);
  ASSERT(frequency <= 0.5f);

  sawtooth_gen_.SetFrequency(frequency);
  normalization_factor_ = 1.0f / (2.0f * frequency);
}

float TriangleDPW::Phase(void) const {
  return sawtooth_gen_.Phase();
}

}  // namespace generators
}  // namespace soundtailor
