/// @file moog_lowaliasnonlinear.h
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

#ifndef SOUNDTAILOR_SRC_FILTERS_MOOG_LOWALIASNONLINEAR_H_
#define SOUNDTAILOR_SRC_FILTERS_MOOG_LOWALIASNONLINEAR_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/filters/filter_base.h"
#include "soundtailor/src/filters/firstorder_polefixedzero.h"

namespace soundtailor {
namespace filters {

/// @brief MoogLowAliasNonLinear low pass filter
class MoogLowAliasNonLinear {
 public:
  MoogLowAliasNonLinear();

  float operator()(float sample);
  Sample operator()(SampleRead sample);
  void SetParameters(const float frequency, const float resonance);

  static const Filter_Meta& Meta(void);

 private:
  // @brief Helper for computing the internal saturation
  float Saturate(float sample);
  // @brief Helper for computing the internal nonlinearity
  float ApplyNonLinearity(float sample);

  alignas(16) FirstOrderPoleFixedZero filters_[4];
  float frequency_;
  float resonance_;
  float last_;
  float last_side_factor_;
};

}  // namespace filters
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_FILTERS_MOOG_LOWALIASNONLINEAR_H_
