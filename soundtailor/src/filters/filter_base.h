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

/// @brief Struct holding a filter "metadata",
/// e.g. informations on its parameters bounds
///
/// Each filter specialization has its own, static one.
// TODO(gm): find a way to to do this in a 100% static way
struct Filter_Meta {
  Filter_Meta(const float freq_min,
              const float freq_passthrough,
              const float freq_max,
              const float res_min,
              const float res_passthrough,
              const float res_max,
              const unsigned int output_delay,
              const float output_gain)
    : freq_min(freq_min),
      freq_passthrough(freq_passthrough),
      freq_max(freq_max),
      res_min(res_min),
      res_passthrough(res_passthrough),
      res_max(res_max),
      output_delay(output_delay),
      output_gain(output_gain) {
    // TODO(gm): These have to be static asserts
    SOUNDTAILOR_ASSERT(freq_min >= 0.0f);
    SOUNDTAILOR_ASSERT(freq_max >= 0.0f);
    SOUNDTAILOR_ASSERT(freq_min < freq_max);
    SOUNDTAILOR_ASSERT(freq_passthrough >= freq_min);
    SOUNDTAILOR_ASSERT(freq_passthrough <= freq_max);
    SOUNDTAILOR_ASSERT(res_min >= 0.0f);
    SOUNDTAILOR_ASSERT(res_max >= 0.0f);
    SOUNDTAILOR_ASSERT(res_min < res_max);
    SOUNDTAILOR_ASSERT(res_passthrough >= res_min);
    SOUNDTAILOR_ASSERT(res_passthrough <= res_max);
    // Arbitrary value here, just as a sanity check
    SOUNDTAILOR_ASSERT(output_delay <= 4);
    SOUNDTAILOR_ASSERT(output_gain > 0.0f);
    // Arbitrary value here, just as a sanity check
    SOUNDTAILOR_ASSERT(output_gain < 10.0f);
  }

  const float freq_min;  ///< Lower bound for filter frequency
  const float freq_passthrough;  ///< Frequency to set the filter to in order
                                 ///< for it to be passthrough
  const float freq_max;  ///< Higher bound for filter frequency
  const float res_min;  ///< Lower bound for filter resonance
  const float res_passthrough;  ///< Resonance to set the filter to in order
                                ///< for it to be passthrough
  const float res_max;  ///< Higher bound for filter resonance
  const unsigned int output_delay;  ///< Delay between input and output
  const float output_gain;  ///< Gain introduced by the filter

 private:
  // No assignment operator for this class
  Filter_Meta& operator=(const Filter_Meta& right);
};

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

/// @brief Actual process function for many samples
///
/// In a context of dynamic polymorphism this will save you from per-sample
/// virtual function calls
/// The compiler should be able to inline it
/// obviously the instance has to be known at compile time
template <typename FilterType>
void ProcessBlock(BlockIn in,
                  BlockOut out,
                  unsigned int block_size,
                  FilterType&& filter_instance) {
  const float* in_ptr(in);
  float* out_write(out);
  for (unsigned int i(0); i < block_size; i += SampleSize) {
    const Sample kInput(VectorMath::Fill(in_ptr));
    VectorMath::Store(out_write, filter_instance(kInput));
    in_ptr += SampleSize;
    out_write += SampleSize;
  }
}

}  // namespace filters
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_FILTERS_FILTER_BASE_H_
