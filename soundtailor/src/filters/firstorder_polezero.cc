/// @file firstorder_polezero.cc
/// @brief Low Pass using a simple 1st order pole-zero filter
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

#include "soundtailor/src/maths.h"

#include "soundtailor/src/filters/firstorder_polezero.h"

namespace soundtailor {
namespace filters {

FirstOrderPoleZero::FirstOrderPoleZero()
    : coeff_(0.0),
      last_(0.0f) {
  // Nothing to do here for now
}

Sample FirstOrderPoleZero::operator()(SampleRead sample) {
  const Sample direct_v(VectorMath::MulConst(static_cast<float>(coeff_ / 2.0f), sample));

  const float actual_coeff = static_cast<float>(1.0 - coeff_);
  float out_v[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  float last = last_;

  for (unsigned int idx = 0; idx < 4; ++idx) {
    // @todo(gm) : implement compile-time unrolling
    const float direct = VectorMath::GetByIndex(direct_v, idx);
    out_v[idx] = direct + last;
    last = out_v[idx] * actual_coeff + direct;
  }
  last_ = last;

  return VectorMath::Fill(out_v[0], out_v[1], out_v[2], out_v[3]);
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
                                 0,
                                 1.0f);
  return metas;
}

}  // namespace filters
}  // namespace soundtailor
