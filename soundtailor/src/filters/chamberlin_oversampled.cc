/// @file chamberlingoversampled.cc
/// @brief Oversampled version of the Chamberlin state variable filter
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

// std::sin, std::cos
#include <cmath>
// std::min
#include <algorithm>

#include "soundtailor/src/maths.h"

#include "soundtailor/src/filters/chamberlin_oversampled.h"

namespace soundtailor {
namespace filters {

ChamberlinOversampled::ChamberlinOversampled()
    : Chamberlin() {
  // Nothing to do here for now
}

Sample ChamberlinOversampled::operator()(SampleRead sample) {
  Chamberlin::operator()(sample);
  return Chamberlin::operator()(sample);
}

void ChamberlinOversampled::SetParameters(const float frequency,
                               const float resonance) {
  SOUNDTAILOR_ASSERT(frequency >= Meta().freq_min);
  SOUNDTAILOR_ASSERT(frequency <= Meta().freq_max);
  SOUNDTAILOR_ASSERT(resonance >= Meta().res_min);
  SOUNDTAILOR_ASSERT(resonance <= Meta().res_max);
  // Stability assertion
  SOUNDTAILOR_ASSERT(frequency * frequency
                     + 2.0f * resonance * frequency < 4.0f);

  damping_ = std::min(resonance, 2.0f - frequency);
  frequency_ = frequency * (1.22f - 0.22f * frequency * damping_);
}

const Filter_Meta& ChamberlinOversampled::Meta(void) {
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

FILTER_PROCESSBLOCK_IMPLEMENTATION(ChamberlinOversampled)

}  // namespace filters
}  // namespace soundtailor
