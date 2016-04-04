/// @file chamberlingoversampled.cc
/// @brief Oversampled version of the Chamberlin state variable filter
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

// std::sin, std::cos
#include <cmath>
// std::min
#include <algorithm>

#include "soundtailor/src/maths.h"

#include "soundtailor/src/filters/chamberlin_oversampled.h"

namespace soundtailor {
namespace filters {

ChamberlinOversampled::ChamberlinOversampled()
    : filter_() {
  // Nothing to do here for now
}

Sample ChamberlinOversampled::operator()(SampleRead sample) {
  filter_(sample);
  return filter_(sample);
}

void ChamberlinOversampled::SetParameters(const float frequency,
                                          const float resonance) {
  return filter_.SetParameters(frequency, resonance);
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

}  // namespace filters
}  // namespace soundtailor
