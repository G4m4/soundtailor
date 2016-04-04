/// @file gain.cc
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

#include "soundtailor/src/maths.h"

#include "soundtailor/src/filters/gain.h"

namespace soundtailor {
namespace filters {

Gain::Gain()
    : gain_(0.0f) {
  // Nothing to do here for now
}

Sample Gain::operator()(SampleRead sample) {
  return VectorMath::MulConst(gain_, sample);
}

void Gain::SetParameters(const float frequency,
                         const float resonance) {
  IGNORE(resonance);
  // Frequency used as a gain
  SOUNDTAILOR_ASSERT(frequency >= Meta().freq_min);
  SOUNDTAILOR_ASSERT(frequency <= Meta().freq_max);
  gain_ = frequency;
}

const Filter_Meta& Gain::Meta(void) {
  static const Filter_Meta metas(0.0f,
                                 1.0f,
                                 1.0f,
                                 0.0f,
                                 1.0f,
                                 1.0f,
                                 0,
                                 1.0f);
  return metas;
}

}  // namespace filters
}  // namespace soundtailor
