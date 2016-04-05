/// @file sawtooth_bl.h
/// @brief Sawtooth signal generator using a BLIT-based algorithm
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

#ifndef SOUNDTAILOR_SRC_GENERATORS_SAWTOOTH_BLIT_H_
#define SOUNDTAILOR_SRC_GENERATORS_SAWTOOTH_BLIT_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/generators/generators_common.h"

namespace soundtailor {
namespace generators {

/// @brief Sawtooth signal generator
/// using band limited imppulse train-based (BLIT) algorithm
class SawtoothBLIT {
public:
  explicit SawtoothBLIT(const float phase = 0.0f);

  Sample operator()(void);
  void SetPhase(const float phase);
  void SetFrequency(const float frequency);
  float ProcessParameters(void);

private:
  /// @brief The left side of a band limited sawtooth segment
  static const float* GetSegment();
  Sample ReadTable(SampleRead value) const;

  PhaseAccumulator sawtooth_gen_;  //< Internal basic sawtooth signal generator
  float alpha_;  //< Table lookup threshold
  float phase_;  //< The expected phase
};

}  // namespace generators
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_GENERATORS_SAWTOOTH_BLIT_H_
