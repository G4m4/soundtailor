/// @file filter_base.h
/// @brief SoundTailor filters base class declaration
///
/// Base interface and common methods for all filters
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

#ifndef SOUNDTAILOR_SRC_FILTERS_FILTER_BASE_H_
#define SOUNDTAILOR_SRC_FILTERS_FILTER_BASE_H_

#include "soundtailor/src/common.h"

namespace soundtailor {
namespace filters {

/// @brief Base class, defining common methods
/// to be implemented in all filters
///
/// All filters are "dumb" - they do not do any fancy parameter management.
/// It means that any asynchronous parameter update has to be managed upstream,
/// because the setters here are "instantaneous".
class Filter_Base {
 public:
  /// @brief Default constructor
  Filter_Base(void) {
    // Nothing to do here for now
  }

  virtual ~Filter_Base() {
    // Nothing to do here for now
  }

  /// @brief Actual process function for one sample
  ///
  /// Process is done per-sample for flexibility purpose, beware of not having
  /// one of this called per-sample! Check in the final code that it actually
  /// gets inlined if needed
  virtual Sample operator()(SampleRead sample) = 0;

  /// @brief Set the filter parameters:
  /// quality factor ("resonance") and cutoff frequency
  ///
  /// Resonance is not normalized
  ///
  /// Frequency is normalized - the input value should be in [0.0f ; 0.5f]
  /// 0.5 because:
  /// f_{n} = \frac{f}{f_{s}}
  /// and:
  /// f_{max} = \frac{f_{s}}{2}
  ///
  /// @param[in]  frequency   Cutoff frequency to set the filter to
  /// @param[in]  resonance   Resonance to set the filter to
  virtual void SetParameters(const float frequency, const float resonance) = 0;
};

}  // namespace filters
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_FILTERS_FILTER_BASE_H_
