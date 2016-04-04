/// @file moog.cc
/// @brief Implementation of a Moog filter
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

#include "soundtailor/src/filters/moog.h"

namespace soundtailor {
namespace filters {

Moog::Moog()
    : filters_(),
      frequency_(0.0f),
      resonance_(0.0f),
      last_(0.0f) {
  // Nothing to do here for now
}

Sample Moog::operator()(SampleRead sample) {
  float out_v[4];
  for (unsigned int i = 0; i < SampleSize; ++i) {
    // @todo (gm) static unrolling
    const float current_sample(VectorMath::GetByIndex(sample, i));
    const float actual_input(current_sample - resonance_ * last_);
    // Todo(gm): find a more efficient way to do that
    float tmp_filtered(actual_input);
    for (MoogLowPassBlock& filter : filters_) {
      tmp_filtered = filter(tmp_filtered);
    }
    last_ = tmp_filtered;
    out_v[i] = tmp_filtered;
  }

  return VectorMath::Fill(out_v[0], out_v[1], out_v[2], out_v[3]);
}

void Moog::SetParameters(const float frequency, const float resonance) {
  SOUNDTAILOR_ASSERT(frequency >= Meta().freq_min);
  SOUNDTAILOR_ASSERT(frequency <= Meta().freq_max);
  SOUNDTAILOR_ASSERT(resonance >= Meta().res_min);
  SOUNDTAILOR_ASSERT(resonance <= Meta().res_max);
  const float temp(frequency * (1.0f + 0.03617f * frequency
                                * (4.0f - resonance) * (4.0f - resonance)));
  frequency_ = 1.25f * temp * (1.0f - 0.595f * temp + 0.24f * temp * temp);
  resonance_ = resonance * (1.0f
                            + 0.077f * frequency_
                            - 0.117f * frequency_ * frequency_
                            - 0.049f * frequency_ * frequency_ * frequency_);
  for (MoogLowPassBlock& filter : filters_) {
    filter.SetParameters(frequency_, resonance_);
  }
}

const Filter_Meta& Moog::Meta(void) {
  static const Filter_Meta metas(1e-5f,
                                 1.0f,
                                 1.0f,
                                 0.0f,
                                 0.0f,
                                 3.9999f,
                                 0,
                                 1.0f);
  return metas;
}

}  // namespace filters
}  // namespace soundtailor
