/// @file adsd.h
/// @brief Envelop generator using Attack-Decay-Sustain-Decay (ADSD) model
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

#ifndef SOUNDTAILOR_SRC_MODULATORS_ADSD_H_
#define SOUNDTAILOR_SRC_MODULATORS_ADSD_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/modulators/modulators_common.h"

namespace soundtailor {
namespace modulators {

/// @brief 2nd order low pass filter
/// using the most simple (and computationally efficient) implementation
class Adsd {
 public:
  Adsd();

  void TriggerOn(void);
  void TriggerOff(void);

  float ComputeOneSample(void);
  Sample operator()(void);

  /// Note that the release here is not used since this is a ADSD:
  /// the decay setting also sets the release
  void SetParameters(const unsigned int attack,
                     const unsigned int decay,
                     const unsigned int release,
                     const float sustain_level);

  Section GetCurrentSection(void) const;

 private:
  /// @brief Helper function for computing the increment at each increment,
  /// given the rise (vertical change) and run (horizontal change)
  double ComputeIncrement(const float rise, const unsigned int run);

  double current_increment_;  ///< Increment to use for the current slope
  double current_value_;  ///< Current amplitude
  Section current_section_;  ///< The current part of the generated envelop
  float sustain_level_;  ///< Amplitude to maintain while sustain is on
  unsigned int cursor_;  ///< Time cursor (e.g. samples since the last event)
  unsigned int attack_;  ///< Time setting for the attack
  unsigned int decay_;  ///< Time setting for the decay
  unsigned int actual_decay_;  ///< Actual decay time in local frame
  unsigned int actual_release_;  ///<  Actual release time in local frame
};

}  // namespace modulators
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_MODULATORS_ADSD_H_
