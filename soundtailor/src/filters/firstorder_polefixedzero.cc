/// @file firstorder_polefixedzero.cc
/// @brief 1st order pole-zero filter, with a fixed zero
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

#include "soundtailor/src/filters/firstorder_polefixedzero.h"

namespace soundtailor {
namespace filters {

FirstOrderPoleFixedZero::FirstOrderPoleFixedZero()
    : pole_coeff_(0.0f),
      zero_coeff_(0.3f),
      last_(0.0f) {
  // Nothing to do here for now
}

Sample FirstOrderPoleFixedZero::operator()(SampleRead sample) {
  const Sample direct_v(VectorMath::MulConst(static_cast<float>(pole_coeff_ / 2.0f), sample));

  const float actual_pole_coeff = static_cast<float>(1.0 - pole_coeff_);
  const float actual_zero_coeff = zero_coeff_;
  float out_v[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  float last = last_;

  for (unsigned int idx = 0; idx < 4; ++idx) {
    // @todo(gm) : implement compile-time unrolling
    const float direct = VectorMath::GetByIndex(direct_v, idx);
    out_v[idx] = direct + last;
    last = out_v[idx] * actual_pole_coeff + actual_zero_coeff * direct;
  }
  last_ = last;

  return VectorMath::Fill(out_v[0], out_v[1], out_v[2], out_v[3]);
}

float FirstOrderPoleFixedZero::operator()(float sample) {
  const float direct(static_cast<float>(pole_coeff_ / 2.0f) * sample);

  const float actual_pole_coeff = static_cast<float>(1.0 - pole_coeff_);
  const float actual_zero_coeff = zero_coeff_;
  float out = 0.0f;
  float last = last_;

  out = direct + last;
  last = out * actual_pole_coeff + actual_zero_coeff * direct;
  last_ = last;

  return out;
}

void FirstOrderPoleFixedZero::SetParameters(const float frequency,
                                     const float resonance) {
  SOUNDTAILOR_ASSERT(frequency >= Meta().freq_min);
  SOUNDTAILOR_ASSERT(frequency <= Meta().freq_max);
  IGNORE(resonance);
  pole_coeff_ = frequency;
}

const Filter_Meta& FirstOrderPoleFixedZero::Meta(void) {
  static const Filter_Meta metas(1e-5f,
                                 1.3f,
                                 1.31f,
                                 0.0f,
                                 0.0f,
                                 3.9999f,
                                 0,
                                 1.3f);
  return metas;
}

}  // namespace filters
}  // namespace soundtailor
