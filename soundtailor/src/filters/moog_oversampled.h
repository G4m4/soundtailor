/// @file moog_oversampled.h
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

#ifndef SOUNDTAILOR_SRC_FILTERS_MOOG_OVERSAMPLED_H_
#define SOUNDTAILOR_SRC_FILTERS_MOOG_OVERSAMPLED_H_

#include <array>

#include "soundtailor/src/filters/moog_lowaliasnonlinear.h"
#include "soundtailor/src/filters/moog_lowaliasnonlinear_lowpassblock.h"

namespace soundtailor {
namespace filters {

/// @brief MoogOversampled low pass filter
class MoogOversampled : public MoogLowAliasNonLinear {
 public:
  MoogOversampled();
  virtual ~MoogOversampled() {
    // Nothing to do here for now
  }
  virtual Sample operator()(SampleRead sample);

  static const Filter_Meta& Meta(void);

 protected:
  std::array<float, 4> history_;  ///< Filter history (last outputs)
                      ///< organized as follows:
                      ///< [y(n) y(n-1) y(n-2) y(n-3)]
};

}  // namespace filters
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_FILTERS_MOOG_OVERSAMPLED_H_