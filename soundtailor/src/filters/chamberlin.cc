/// @file chamberlin.cc
/// @brief Low Pass filter using a Chamberlin state variable filter
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

#include "soundtailor/src/maths.h"

#include "soundtailor/src/filters/chamberlin.h"

namespace soundtailor {
namespace filters {

Chamberlin::Chamberlin()
    : Filter_Base(),
      lp_(0.0f),
      hp_(0.0f),
      bp_(0.0f),
      frequency_(0.0f),
      damping_(0.0f) {
  // Nothing to do here for now
}

Sample Chamberlin::operator()(SampleRead sample) {
  float out[soundtailor::SampleSize];
  for (unsigned int i(0); i < soundtailor::SampleSize; ++i) {
    lp_ = frequency_ * bp_ + lp_;
    hp_ = GetByIndex(sample, i) - lp_ - bp_ * damping_;
    bp_ = frequency_ * hp_ + bp_;

    out[i] = lp_;
  }

  return Fill(&out[0]);
}

void Chamberlin::SetParameters(const float frequency,
                               const float resonance) {
  SOUNDTAILOR_ASSERT(resonance > 0.0f);
  SOUNDTAILOR_ASSERT(resonance < 2.0f);
  SOUNDTAILOR_ASSERT(frequency > 0.0f);
  SOUNDTAILOR_ASSERT(frequency < 2.0f);
  // Stability assertion
  SOUNDTAILOR_ASSERT(frequency * frequency + 2.0f * resonance * frequency < 4.0f);

  frequency_ = static_cast<float>(2.0 * std::sin(Pi * static_cast<double>(frequency)));
  damping_ = 1.0f / resonance;
}

}  // namespace filters
}  // namespace soundtailor
