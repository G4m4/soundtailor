/// @file moog_lowaliasnonlinear_lowpassblock.cc
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

#include "soundtailor/src/filters/moog_lowaliasnonlinear_lowpassblock.h"

namespace soundtailor {
namespace filters {

MoogLowAliasNonLinearLowPassBlock::MoogLowAliasNonLinearLowPassBlock()
    : Filter_Base(),
      pole_coeff_(0.0f),
      zero_coeff_(0.3f),
      last_(0.0f) {
  // Nothing to do here for now
}

Sample MoogLowAliasNonLinearLowPassBlock::operator()(SampleRead sample) {
  const float kHistoryGain(1.0f - pole_coeff_);
  const float direct = pole_coeff_ * sample;
  const float out = direct + last_;

  last_ = direct * (kHistoryGain + zero_coeff_) + kHistoryGain * last_;

  return out;
}

void MoogLowAliasNonLinearLowPassBlock::SetParameters(const float frequency,
                                     const float resonance) {
  SOUNDTAILOR_ASSERT(frequency >= Meta().freq_min);
  SOUNDTAILOR_ASSERT(frequency <= Meta().freq_max);
  IGNORE(resonance);
  pole_coeff_ = frequency;
}

const Filter_Meta& MoogLowAliasNonLinearLowPassBlock::Meta(void) {
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

FILTER_PROCESSBLOCK_IMPLEMENTATION(MoogLowAliasNonLinearLowPassBlock)

}  // namespace filters
}  // namespace soundtailor
