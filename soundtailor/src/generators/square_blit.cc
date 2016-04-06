/// @file square_bl.cc
/// @brief Square signal generator using BLIT algorithm - implementation
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

#include "soundtailor/src/generators/square_blit.h"
#include "soundtailor/src/maths.h"

namespace soundtailor {
namespace generators {

SquareBLIT::SquareBLIT(const float phase)
    : sawtooth1_(phase),
      sawtooth2_(phase) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
  SetPhase(phase);
  ProcessParameters();
}

Sample SquareBLIT::operator()(void) {
  const Sample reference(sawtooth1_());
  const Sample phased(sawtooth2_());
  const Sample out(VectorMath::Add(reference, VectorMath::MulConst(-1.0f, phased)));

  return out;
}

void SquareBLIT::SetPhase(const float phase) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
  sawtooth1_.SetPhase(phase);
  // Fixed pulse width set here:
  sawtooth2_.SetPhase(phase + 1.0f > 1.0f ? phase - 1.0f : phase + 1.0f);
}

void SquareBLIT::SetFrequency(const float frequency) {
  SOUNDTAILOR_ASSERT(frequency >= 0.0f);
  SOUNDTAILOR_ASSERT(frequency <= 0.5f);

  sawtooth1_.SetFrequency(frequency);
  sawtooth2_.SetFrequency(frequency);
}

float SquareBLIT::ProcessParameters(void) {
  const float out1(sawtooth1_.ProcessParameters());
  const float out2(sawtooth2_.ProcessParameters());
  return out1 + out2;
}

}  // namespace generators
}  // namespace soundtailor
