/// @file moog_lowaliasnonlinear.cc
/// @brief Implementation of a Moog Low-Alias nonlinear filter
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

#include "soundtailor/src/filters/moog_lowaliasnonlinear.h"

namespace soundtailor {
namespace filters {

MoogLowAliasNonLinear::MoogLowAliasNonLinear()
    : filters_(),
      frequency_(0.0f),
      resonance_(0.0f),
      last_(0.0f),
      last_side_factor_(0.0f) {
  // Nothing to do here for now
}

float MoogLowAliasNonLinear::operator()(float sample) {
  float actual_input(sample - resonance_ * last_);
  float kCurrentSideFactor(Saturate(last_side_factor_));

  last_side_factor_ = actual_input * actual_input;
  last_side_factor_ *= 0.062f;
  last_side_factor_ += kCurrentSideFactor * 0.993f;

  kCurrentSideFactor = 1.0f
    - kCurrentSideFactor
    + kCurrentSideFactor * kCurrentSideFactor / 2.0f;
  actual_input *= kCurrentSideFactor;

  // @todo(gm): find a more efficient way to do that
  // notice the 1.3 factor
  float tmp_filtered(actual_input);
  tmp_filtered = filters_[1](1.3f * filters_[0](1.3f * tmp_filtered));

  tmp_filtered = ApplyNonLinearity(tmp_filtered);

  tmp_filtered = filters_[3](1.3f * filters_[2](1.3f * tmp_filtered));

  last_ = tmp_filtered;

  return tmp_filtered;
}

Sample MoogLowAliasNonLinear::operator()(SampleRead sample) {
  const Sample direct_v(VectorMath::MulConst(0.18f + 0.25f * resonance_, sample));
  float last = last_;
  float out_v[4];
  for (int i = 0; i < 4; ++i) {
    const float current_sample = VectorMath::GetByIndex(direct_v, i);
    float actual_input(current_sample - resonance_ * last);

    float kCurrentSideFactor(Saturate(last_side_factor_));

    last_side_factor_ = actual_input * actual_input;
    last_side_factor_ *= 0.062f;
    last_side_factor_ += kCurrentSideFactor * 0.993f;

    kCurrentSideFactor = 1.0f
      - kCurrentSideFactor
      + kCurrentSideFactor * kCurrentSideFactor / 2.0f;
    actual_input *= kCurrentSideFactor;

    // @todo(gm): find a more efficient way to do that
    // notice the 2.0 factor
    float tmp_filtered(actual_input);
    tmp_filtered = filters_[1](2.0f * filters_[0](2.0f * tmp_filtered));

    tmp_filtered = ApplyNonLinearity(tmp_filtered);

    tmp_filtered = filters_[3](2.0f * filters_[2](2.0f * tmp_filtered));

    out_v[i] = tmp_filtered;
    last = tmp_filtered;
  }
  last_ = last;

  return VectorMath::Fill(out_v[0], out_v[1], out_v[2], out_v[3]);
}

void MoogLowAliasNonLinear::SetParameters(const float frequency,
                                          const float resonance) {
  SOUNDTAILOR_ASSERT(frequency >= Meta().freq_min);
  SOUNDTAILOR_ASSERT(frequency <= Meta().freq_max);
  SOUNDTAILOR_ASSERT(resonance >= Meta().res_min);
  SOUNDTAILOR_ASSERT(resonance <= Meta().res_max);
  const float kActualResonance(resonance / 4.0f);
  const float temp(frequency * (1.0f + 0.5787f * frequency
                      * (1.0f - kActualResonance) * (1.0f - kActualResonance)));
  frequency_ = 1.25f * temp * (1.0f - 0.595f * temp + 0.24f * temp * temp);
  resonance_ = kActualResonance * (
    1.4f
    + 0.108f * frequency_
    - 0.164f * frequency_ * frequency_
    - 0.069f * frequency_ * frequency_ * frequency_);
  for (FirstOrderPoleFixedZero& filter : filters_) {
    filter.SetParameters(frequency_, resonance_);
  }
}

float MoogLowAliasNonLinear::Saturate(float sample) {
  constexpr Sample lowerBound = { -1.0f, -1.0f, -1.0f, -1.0f };
  constexpr Sample upperBound = { 1.0f, 1.0f, 1.0f, 1.0f };

  const Sample vecSample = VectorMath::Fill(sample);
  const Sample result = VectorMath::Min(VectorMath::Max(vecSample, lowerBound), upperBound);
  return VectorMath::GetFirst(result);
}

float MoogLowAliasNonLinear::ApplyNonLinearity(float sample) {
  constexpr Sample threshold = { 1.0f, 1.0f, 1.0f, 1.0f };

  Sample vecSample = VectorMath::Fill(sample);
  // TODO @gm: collapse predicate to mask
  if (VectorMath::GetFirst(VectorMath::LessEqual(threshold, VectorMath::Abs(vecSample))) != 0.0f) {
    const float kFactor(2.0f / 3.0f);
    return kFactor * Saturate(sample);
  } else {
    return sample - sample * sample * sample / 3.0f;
  }
}

const Filter_Meta& MoogLowAliasNonLinear::Meta(void) {
  static const Filter_Meta metas(1e-5f,
                                 1.0f,
                                 1.0f,
                                 0.0f,
                                 0.0f,
                                 3.9999f,
                                 0,
                                 2.0f);
  return metas;
}

}  // namespace filters
}  // namespace soundtailor
