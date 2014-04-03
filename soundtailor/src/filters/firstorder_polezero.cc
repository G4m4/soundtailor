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
      last_input_(0.0f),
      last_output_(0.0f)
{
  // Nothing to do here for now
}

Sample FirstOrderPoleZero::operator()(SampleRead sample) {
  const Sample direct_v(Add(sample, RotateOnRight(sample, last_input_)));
  const Sample muldirect_v(MulConst(static_cast<float>(coeff_ / 2.0),
                                    direct_v));
  const float comp_coeff(static_cast<float>(1.0 - coeff_));
  const float oldest_out(GetByIndex<0>(muldirect_v) + last_output_ * comp_coeff);

#if (_USE_SSE)
  const float old_out(GetByIndex<1>(muldirect_v) + oldest_out * comp_coeff);
  const float new_out(GetByIndex<2>(muldirect_v) + old_out * comp_coeff);
  const float newest_out(GetByIndex<3>(muldirect_v) + new_out * comp_coeff);

  const Sample out(Fill(oldest_out, old_out, new_out, newest_out));

  last_input_ = GetLast(sample);
  last_output_ = newest_out;
#else
  const Sample out(oldest_out);
  last_input_ = sample;
  last_output_ = oldest_out;
#endif (_USE_SSE)

  return out;
}

void FirstOrderPoleZero::SetParameters(const float frequency,
                                       const float resonance) {
  SOUNDTAILOR_ASSERT(frequency > 0.0f);
  SOUNDTAILOR_ASSERT(frequency <= 0.5f);
  IGNORE(resonance);
  const double lambda(Pi * frequency);
  coeff_ = (2.0 * std::sin(lambda)) / (std::cos(lambda) + std::sin(lambda));
}

}  // namespace filters
}  // namespace soundtailor
