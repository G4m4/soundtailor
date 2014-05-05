/// @file moog_lowpassblock.cc
/// @brief Low Pass (1st order pole-zero filter), base block for Moog filter
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

#include "soundtailor/src/maths.h"

#include "soundtailor/src/filters/moog_lowpassblock.h"

namespace soundtailor {
namespace filters {

MoogLowPassBlock::MoogLowPassBlock()
    : Filter_Base(),
      pole_coeff_(0.0f),
      zero_coeff_(0.3f),
      last_(0.0f) {
  // Nothing to do here for now
}

Sample MoogLowPassBlock::operator()(SampleRead sample) {
  const float kGain(pole_coeff_ / 1.3f);
  const float direct = kGain * sample;
  const float out = direct + last_;

  last_ = out * (1.0f - pole_coeff_) + zero_coeff_ * direct;

  return out;
}

void MoogLowPassBlock::SetParameters(const float frequency,
                                     const float resonance) {
  // TODO(gm): check for these bounds
  SOUNDTAILOR_ASSERT(frequency > 0.0f);
  SOUNDTAILOR_ASSERT(frequency <= 2.0f);
  IGNORE(resonance);
  pole_coeff_ = frequency;
}

const Filter_Meta& MoogLowPassBlock::Meta(void) {
  static const Filter_Meta metas(1e-5f,
                                 1.3f,
                                 1.31f,
                                 0.0f,
                                 0.0f,
                                 3.9999f);  // Arbitrary value
  return metas;
}

}  // namespace filters
}  // namespace soundtailor