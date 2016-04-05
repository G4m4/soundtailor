/// @file secondorder_raw.cc
/// @brief Low Pass filter using a simple ("raw") 2nd order implementation
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

#include "soundtailor/src/filters/secondorder_raw.h"

namespace soundtailor {
namespace filters {

SecondOrderRaw::SecondOrderRaw()
    : gain_(VectorMath::Fill(0.0f)),
      coeffs_{ 0.0f, 0.0f, 0.0f, 0.0f },
      history_{ 0.0f, 0.0f, 0.0f, 0.0f } {
  // Nothing to do here for now
}

Sample SecondOrderRaw::operator()(SampleRead sample) {
  // Direct Form 1 implementation:
  // the Direct Form 2, although usually more efficient, has issues with
  // time-varying parameters

  // Vector = (x_{n}, x_{n + 1}, x_{n + 2}, x_{n + 3})
  // previous = (x_{n - 1}, x_{n}, x_{n + 1}, x_{n + 2})
  // last = (x_{n - 2}, x_{n - 1}, x_{n}, x_{n + 1})

#if 1
  const Sample previous(VectorMath::RotateOnRight(sample, history_[1]));
  const Sample last(VectorMath::RotateOnRight(previous, history_[0]));
  const Sample current(VectorMath::Mul(gain_, sample));
  // previous *= b1
  const Sample previous_gain(VectorMath::MulConst(coeffs_[1], previous));
  // previous *= b2
  const Sample last_gain(VectorMath::MulConst(coeffs_[0], last));
  // All weighted inputs cumulated sum
  const Sample tmp_sum(VectorMath::Add(VectorMath::Add(current, previous_gain), last_gain));

  const float oldest_out(VectorMath::GetByIndex<0>(tmp_sum)
                         + history_[2] * coeffs_[2]
                         + history_[3] * coeffs_[3]);
  const float old_out(VectorMath::GetByIndex<1>(tmp_sum)
                      + history_[3] * coeffs_[2]
                      + oldest_out * coeffs_[3]);
  const float new_out(VectorMath::GetByIndex<2>(tmp_sum)
                      + oldest_out * coeffs_[2]
                      + old_out * coeffs_[3]);
  const float newest_out(VectorMath::GetByIndex<3>(tmp_sum)
                         + old_out * coeffs_[2]
                         + new_out * coeffs_[3]);
  const Sample out(VectorMath::Fill(oldest_out, old_out, new_out, newest_out));
  const Sample history(VectorMath::TakeEachRightHalf(sample, out));
  VectorMath::Store(&history_[0], history);
#else
  // @todo(gm) find out what's going wrong above
  float out_v[4];
  for (unsigned int i = 0; i < SampleSize; ++i) {
    const float current_sample = VectorMath::GetByIndex(sample, i);
    const float out(gain_ * current_sample
                    + history_[0] * coeffs_[0]
                    + history_[1] * coeffs_[1]
                    + history_[2] * coeffs_[2]
                    + history_[3] * coeffs_[3]);
    history_[0] = history_[1];
    history_[1] = current_sample;
    history_[2] = history_[3];
    history_[3] = out;
    out_v[i] = out;
  }
  const Sample out(VectorMath::Fill(out_v[0], out_v[1], out_v[2], out_v[3]));
#endif  // 1

  return out;
}

void SecondOrderRaw::SetParameters(const float frequency,
                                   const float resonance) {
  // Based on Audio EQ Cookbook material
  SOUNDTAILOR_ASSERT(frequency >= Meta().freq_min);
  SOUNDTAILOR_ASSERT(frequency <= Meta().freq_max);
  SOUNDTAILOR_ASSERT(resonance >= Meta().res_min);
  SOUNDTAILOR_ASSERT(resonance <= Meta().res_max);

  // Computations done in double since precision is crucial here
  const double kFrequency(frequency);
  const double kResonance(resonance);

  const double kOmega(2.0 * Pi * kFrequency);
  const double kSinOmega(std::sin(kOmega));
  const double kCosOmega(std::cos(kOmega));
  const double kAlpha(kSinOmega / (2.0 * kResonance));

  // Actual coefficients
  const double b0 = (1.0 - kCosOmega) / 2.0;
  const double b1 = (1.0 - kCosOmega);
  const double b2 = (1.0 - kCosOmega) / 2.0;
  const double a0 = 1.0 + kAlpha;
  const double a1 = -2.0 * kCosOmega;
  const double a2 = 1.0 - kAlpha;

  // Assigning normalized coefficients
  gain_ = VectorMath::Fill(static_cast<float>(b0 / a0));
  coeffs_[0] = static_cast<float>(b2 / a0);
  coeffs_[1] = static_cast<float>(b1 / a0);
  coeffs_[2] = static_cast<float>(-a2 / a0);
  coeffs_[3] = static_cast<float>(-a1 / a0);
}

const Filter_Meta& SecondOrderRaw::Meta(void) {
  static const Filter_Meta metas(1e-5f,
                                 0.4999f,
                                 0.4999f,
                                 0.0f,
                                 0.7f,
                                 1000.0f,
                                 0,
                                 1.0f);
  return metas;
}

}  // namespace filters
}  // namespace soundtailor
