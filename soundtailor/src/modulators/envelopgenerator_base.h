/// @file envelopgenerator_base.h
/// @brief SoundTailor envelop generators common interface declaration
///
/// Base interface and common methods for all envelop generators
///
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

#ifndef SOUNDTAILOR_SRC_MODULATORS_ENVELOPGENERATOR_BASE_H_
#define SOUNDTAILOR_SRC_MODULATORS_ENVELOPGENERATOR_BASE_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/modulators/modulators_common.h"

namespace soundtailor {
namespace modulators {

/// @brief Base class, defining common methods
/// to be implemented in all envelop generators
///
/// All envelop generators are "dumb" - they do not do any fancy
/// parameter management.
/// It means that any asynchronous parameter update has to be managed upstream,
/// because the setters here are "instantaneous".
class EnvelopGenerator_Base {
 public:
  /// @brief Default constructor
  EnvelopGenerator_Base(void) {
    // Nothing to do here for now
  }

  virtual ~EnvelopGenerator_Base() {
    // Nothing to do here for now
  }

  /// @brief Event for triggering the beginning of the envelop
  ///
  /// Note that the amplitude start value is the one applied when triggering.
  /// e.g. repeated calls to this function will lead to accumulated envelops.
  virtual void TriggerOn(void) = 0;

  /// @brief Event for triggering the end of the envelop
  ///
  /// Note that this does not mean that the envelop stops right after this
  /// function was called, depending on the parameters currently in use
  virtual void TriggerOff(void) = 0;

  /// @brief Actual process function for one sample:
  /// output one sample of the envelop
  ///
  /// Process is done per-sample for flexibility purpose, beware of not having
  /// one of this called per-sample! Check in the final code that it actually
  /// gets inlined if needed
  virtual Sample operator()(void) = 0;

  /// @brief Set the envelop generator parameters,
  /// common to all generators implementations
  ///
  /// @param[in]  attack    Length of the attack (absolute value in samples)
  /// @param[in]  decay    Length of the decay (absolute value in samples)
  /// @param[in]  release    Length of the release (absolute value in samples)
  /// @param[in]  sustain_level    Sustain amplitude (absolute normalized value)
  virtual void SetParameters(const unsigned int attack,
                             const unsigned int decay,
                             const unsigned int release,
                             const float sustain_level) = 0;

  /// @brief Modulator section getter
  ///
  /// @return the envelop current state
  virtual Section GetCurrentSection(void) const = 0;
};

}  // namespace modulators
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_MODULATORS_ENVELOPGENERATOR_BASE_H_
