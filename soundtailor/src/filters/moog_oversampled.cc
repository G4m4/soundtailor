/// @file moog_oversampled.cc
/// @brief Implementation of a properly oversampled Moog filter
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

#include "soundtailor/src/filters/moog_oversampled.h"

namespace soundtailor {
namespace filters {

MoogOversampled::MoogOversampled()
    : filter_(),
      history_{0.0f, 0.0f, 0.0f, 0.0f},
      last_(0.0f) {
  // Nothing to do here for now
}

float MoogOversampled::operator()(float sample) {
  const Sample kHistoryCoeffs(VectorMath::Fill( 0.19f, 0.57f, 0.57f, 0.19f ));
  filter_(sample);
  // 2x oversampled
  const float kOut(filter_(sample));
  const Sample kHistory(VectorMath::Fill(&history_[0]));
  const Sample kNewHistory(VectorMath::RotateOnRight(kHistory, kOut));
  const float kTemp(VectorMath::AddHorizontal(VectorMath::Mul(kNewHistory, kHistoryCoeffs)));

  const float out(kTemp + 0.52f * last_);
  VectorMath::Store(&history_[0], kNewHistory);
  last_ = out;

  return out;
}

Sample MoogOversampled::operator()(SampleRead sample) {
  // @todo(gm) find out a better way to do that
  float out_v[4];
  float direct_v[4];
  VectorMath::Store(&direct_v[0], sample);
  out_v[0] = MoogOversampled::operator()(direct_v[0]);
  out_v[1] = MoogOversampled::operator()(direct_v[1]);
  out_v[2] = MoogOversampled::operator()(direct_v[2]);
  out_v[3] = MoogOversampled::operator()(direct_v[3]);

  const Sample out(VectorMath::Fill(out_v[0], out_v[1], out_v[2], out_v[3]));
  return out;
}

void MoogOversampled::SetParameters(const float frequency,
                                    const float resonance) {
  filter_.SetParameters(frequency, resonance);
}

const Filter_Meta& MoogOversampled::Meta(void) {
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
