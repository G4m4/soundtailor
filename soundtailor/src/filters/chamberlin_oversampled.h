/// @file chamberlingoversampled.h
/// @brief Oversampled version of the Chamberlin state variable filter
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

#ifndef SOUNDTAILOR_SRC_FILTERS_CHAMBERLIN_OVERSAMPLED_H_
#define SOUNDTAILOR_SRC_FILTERS_CHAMBERLIN_OVERSAMPLED_H_

#include "soundtailor/src/filters/chamberlin.h"

namespace soundtailor {
namespace filters {

/// @brief Oversampled Chamberlin state variable low pass filter
class ChamberlinOversampled {
 public:
  ChamberlinOversampled();

  Sample operator()(SampleRead sample);
  void SetParameters(const float frequency, const float resonance);

  static const Filter_Meta& Meta(void);

private:
  alignas(16) Chamberlin filter_;
};

}  // namespace filters
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_FILTERS_CHAMBERLIN_OVERSAMPLED_H_
