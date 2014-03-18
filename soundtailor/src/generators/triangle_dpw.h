/// @file triangle_dpw.h
/// @brief Triangle signal generator using DPW algorithm
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

#ifndef SOUNDTAILOR_SRC_GENERATORS_TRIANGLE_DPW_H_
#define SOUNDTAILOR_SRC_GENERATORS_TRIANGLE_DPW_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/generators/generators_common.h"

namespace soundtailor {
namespace generators {

/// @brief Triangle signal generator
/// using Differentiated Parabolic Wave (DPW) algorithm
class TriangleDPW : public Generator_Base {
 public:
  explicit TriangleDPW(const float phase = 0.0f);
  virtual ~TriangleDPW() {
    // Nothing to do here for now
  }
  virtual Sample operator()(void);
  virtual void SetPhase(const float phase);
  virtual void SetFrequency(const float frequency);
  virtual float Phase(void) const;

 protected:
  PhaseAccumulator sawtooth_gen_;  //< Internal basic sawtooth signal generator
  Differentiator differentiator_;  //< Internal basic differentiator
  float normalization_factor_;  //< To be applied on the signal after synthesis
};

}  // namespace generators
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_GENERATORS_TRIANGLE_DPW_H_
