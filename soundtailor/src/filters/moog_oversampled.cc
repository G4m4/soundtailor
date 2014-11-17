/// @file moog_oversampled.cc
/// @brief Implementation of a properly oversampled Moog filter
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

#include "soundtailor/src/filters/moog_oversampled.h"

namespace soundtailor {
namespace filters {

MoogOversampled::MoogOversampled()
    : MoogLowAliasNonLinear(),
    history_({{0.0f, 0.0f, 0.0f, 0.0f}}) {
  // Nothing to do here for now
}

Sample MoogOversampled::operator()(SampleRead sample) {
  const Sample kFirstOut(MoogLowAliasNonLinear::operator()(sample));
  history_[3] = history_[2];
  history_[2] = history_[1];
  history_[1] = kFirstOut;
  // 2x oversampled
  const Sample kSecondOut(MoogLowAliasNonLinear::operator()(sample));
  const Sample kTemp(history_[3]);
  history_[3] = history_[2];
  history_[2] = history_[1];
  history_[1] = kSecondOut;

  const Sample out(0.19f * kTemp
                   + 0.57f * history_[3]
                   + 0.57f * history_[2]
                   + 0.19f * history_[1]);

  history_[0] = history_[0] * -0.52f + out;

  return out;
}

const Filter_Meta& MoogOversampled::Meta(void) {
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
