/// @file chamberlin.h
/// @brief Low Pass filter using a Chamberlin state variable filter
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

#ifndef SOUNDTAILOR_SRC_FILTERS_CHAMBERLIN_H_
#define SOUNDTAILOR_SRC_FILTERS_CHAMBERLIN_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/filters/filter_base.h"

namespace soundtailor {
namespace filters {

/// @brief Chamberlin state variable low pass filter
class Chamberlin {
 public:
  Chamberlin();

  Sample operator()(SampleRead sample);
  void SetParameters(const float frequency, const float resonance);

  static const Filter_Meta& Meta(void);

 private:
  float lp_;
  float bp_;
  float frequency_;
  float damping_;
};

}  // namespace filters
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_FILTERS_CHAMBERLIN_H_
