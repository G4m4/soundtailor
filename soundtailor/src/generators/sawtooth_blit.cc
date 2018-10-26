/// @file sawtooth_blit.cc
/// @brief Sawtooth signal generator using BLIT algorithm - implementation
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

#include "soundtailor/src/generators/sawtooth_blit.h"

namespace soundtailor {
namespace generators {

SawtoothBLIT::SawtoothBLIT(const float phase)
    : sawtooth_gen_(),
      alpha_(0.0f),
      phase_(0.0f) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
  SetPhase(phase);
  ProcessParameters();
}

Sample SawtoothBLIT::operator()(void) {
  const Sample current(sawtooth_gen_());
  const Sample phase(VectorMath::Fill(phase_));
  // Phase input here
  const Sample A(VectorMath::IncrementAndWrap(current, phase));
  const Sample C(ReadTable(A));
  const Sample B(VectorMath::IncrementAndWrap(A, VectorMath::Fill(1.0)));
  const Sample out(VectorMath::Add(B, C));

  return out;
}

void SawtoothBLIT::SetPhase(const float phase) {
  SOUNDTAILOR_ASSERT(phase <= 1.0f);
  SOUNDTAILOR_ASSERT(phase >= -1.0f);
  sawtooth_gen_.SetPhase(1.0);
  phase_ = phase;
}

void SawtoothBLIT::SetFrequency(const float frequency) {
  SOUNDTAILOR_ASSERT(frequency >= 0.0f);
  SOUNDTAILOR_ASSERT(frequency <= 0.5f);

  sawtooth_gen_.SetFrequency(frequency);
  alpha_ = frequency * 4.0f;
}

float SawtoothBLIT::ProcessParameters(void) {
  const float current(sawtooth_gen_.ProcessParameters());
  //const float squared(current * current);
  //return normalization_factor_ * differentiator_.ProcessParameters(squared);
  return current;
}

const float* SawtoothBLIT::GetSegment() {
  static const float kSegment[] = {
#include "soundtailor/src/generators/blsawtooth_segment.inc"
  };

  // Simple sanity check
  SOUNDTAILOR_ASSERT(sizeof(kSegment) / sizeof(float) == 5400);

  return &kSegment[0];
}

Sample SawtoothBLIT::ReadTable(SampleRead value) const {
  const Sample abs_value(VectorMath::Abs(value));
  const Sample sign_value(VectorMath::Sgn(value));
  // @todo(gm) get rid of that hardcoded value
  // (whenever we actually compute the tables on the fly)
  const Sample kZero(VectorMath::Fill(0.0f));
  const Sample kHalfM(VectorMath::Fill(5400.0f));
  const Sample kAlphaInverse(VectorMath::Fill(1.0f / alpha_));

  // relative_index = kHalfM * abs_value / alpha
  const Sample relative_index(VectorMath::Mul(kHalfM,
                                              VectorMath::Mul(abs_value, kAlphaInverse)));
  // index = kHalfM - relative_index - 1
  const Sample unbounded_index(VectorMath::Sub(kHalfM,
                                               VectorMath::Add(relative_index, VectorMath::Fill(1))));

  // Values outside [-alpha ; alpha] would get out of bounds results, clipping
  const SampleInt index(VectorMath::TruncToInt(VectorMath::Clamp(unbounded_index, kZero, kHalfM)));
  alignas(16) float tmp_v[4];
  const float* kTable(GetSegment());
  tmp_v[0] = kTable[VectorMath::GetByIndex<0>(index)];
  tmp_v[1] = kTable[VectorMath::GetByIndex<1>(index)];
  tmp_v[2] = kTable[VectorMath::GetByIndex<2>(index)];
  tmp_v[3] = kTable[VectorMath::GetByIndex<3>(index)];
  const Sample tmp(VectorMath::Fill(&tmp_v[0]));

  // if abs_value < alpha_
  //  return sign_value * tmp
  // else
  //  return 0.0
  const Sample mask(VectorMath::LessThan(abs_value, VectorMath::Fill(alpha_)));
  const Sample factor(VectorMath::ExtractValueFromMask(sign_value, mask));
  const Sample out(VectorMath::Mul(factor, tmp));
  return out;
}

}  // namespace generators
}  // namespace soundtailor
