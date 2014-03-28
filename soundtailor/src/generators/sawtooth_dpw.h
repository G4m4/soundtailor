/// @file sawtooth_dpw.h
/// @brief Sawtooth signal generator using DPW algorithm
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

#ifndef SOUNDTAILOR_SRC_GENERATORS_SAWTOOTH_DPW_H_
#define SOUNDTAILOR_SRC_GENERATORS_SAWTOOTH_DPW_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/generators/triangle_dpw.h"

namespace soundtailor {
namespace generators {

/// @brief Sawtooth signal generator
/// using Differentiated Parabolic Wave (DPW) algorithm
class SawtoothDPW : public TriangleDPW {
 public:
  explicit SawtoothDPW(const float phase = 0.0f);
  virtual Sample operator()(void);
  // TODO(gm): Find a better name for this
  virtual float ProcessScalar(void);
  // 1-sample advance required after calling this function!
  // The frequency also needs to be set
  virtual void SetPhase(const float phase);
  virtual void SetFrequency(const float frequency);
};

}  // namespace generators
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_GENERATORS_SAWTOOTH_DPW_H_
