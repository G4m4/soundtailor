/// @file secondorder_raw.h
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

#ifndef SOUNDTAILOR_SRC_FILTERS_SECONDORDER_RAW_H_
#define SOUNDTAILOR_SRC_FILTERS_SECONDORDER_RAW_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/filters/filter_base.h"

namespace soundtailor {
namespace filters {

/// @brief 2nd order low pass filter
/// using the most simple (and computationally efficient) implementation
class SecondOrderRaw {
 public:
  SecondOrderRaw();

  Sample operator()(SampleRead sample);
  void SetParameters(const float frequency, const float resonance);

  static const Filter_Meta& Meta(void);

 private:
  // The following is explicitly expanded into a full 128b register;
  // for alignment reasons the remaining 96b would be wasted anyway
  // Keep that in mind if refactoring
  alignas(16) Sample gain_;  ///< Filter gain (b0 coefficient)
  alignas(16) float coeffs_[4];  ///< Filter coefficients (for zeroes and poles)
                     ///< organized as follows:
                     ///< [b2 b1 -a2 -a1]
  // @todo(gm) fix alignment, this is a mess
  alignas(16) float history_[4];  ///< Filter history (last inputs/outputs)
                      ///< organized as follows:
                      ///< [x(n-2) x(n-1) y(n-2) y(n-1)]
                      ///< where x are the last inputs
                      ///< and y the last outputs
};

}  // namespace filters
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_FILTERS_SECONDORDER_RAW_H_
