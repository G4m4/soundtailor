/// @file firstorder_polezero.cc
/// @brief Low Pass using a simple 1st order pole-zero filter
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

#include "soundtailor/src/filters/firstorder_polezero.h"

namespace soundtailor {
namespace filters {

FirstOrderPoleZero::FirstOrderPoleZero()
    : Filter_Base(),
      coeff_(0.0),
      last_(0.0f) {
  // Nothing to do here for now
}

Sample FirstOrderPoleZero::operator()(SampleRead sample) {
  const float direct(sample * static_cast<float>(coeff_ / 2.0f));
  const float out(direct + last_);

  last_ = out * static_cast<float>(1.0 - coeff_) + direct;

  return out;
}

void FirstOrderPoleZero::SetParameters(const float frequency,
                                       const float resonance) {
  SOUNDTAILOR_ASSERT(frequency >= Meta().freq_min);
  SOUNDTAILOR_ASSERT(frequency <= Meta().freq_max);
  IGNORE(resonance);
  const double lambda(Pi * frequency);
  coeff_ = (2.0 * std::sin(lambda)) / (std::cos(lambda) + std::sin(lambda));
}

const Filter_Meta& FirstOrderPoleZero::Meta(void) {
  static const Filter_Meta metas(1e-5f,
                                 0.5f,
                                 0.5f,
                                 // No resonance anyway
                                 0.0f,
                                 1.0f,
                                 1.0f,
                                 1);
  return metas;
}

}  // namespace filters
}  // namespace soundtailor
