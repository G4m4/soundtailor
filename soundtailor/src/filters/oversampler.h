/// @file oversampler.h
/// @brief A wrapper for naive filter oversampling
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

#ifndef SOUNDTAILOR_SRC_FILTERS_OVERSAMPLER_H_
#define SOUNDTAILOR_SRC_FILTERS_OVERSAMPLER_H_

#include "soundtailor/src/filters/filter_base.h"

namespace soundtailor {
namespace filters {

/// @brief Oversample a filter by repeated calls to its process function
template <typename FilterType>
class Oversampler {
 public:
  Oversampler()
    :  filter_(FilterType()) {
  // Nothing to do here for now
  }

  Sample operator()(SampleRead sample) {
    // The oversampling order should be a (template) argument
    filter_(sample);
    return filter_(sample);
  }

  void ProcessBlock(BlockIn in, BlockOut out, unsigned int block_size) {
    const float* in_ptr(in);
    float* out_write(out);
    for (unsigned int i(0); i < block_size; i += SampleSize) {
      const Sample kInput(VectorMath::Fill(in_ptr));
      const Sample kOutput(static_cast<Oversampler<FilterType>*>(this)->operator()(kInput));
      VectorMath::Store(out_write, kOutput);
      in_ptr += SampleSize;
      out_write += SampleSize;
    }
  }

  void SetParameters(const float frequency, const float resonance) {
    filter_.SetParameters(frequency, resonance);
  }

  static const Filter_Meta& Meta(void) {
    // Same metadata, except the usually reduced delay
    static const Filter_Meta metas(
      FilterType::Meta().freq_min,
      FilterType::Meta().freq_passthrough,
      FilterType::Meta().freq_max,
      FilterType::Meta().res_min,
      FilterType::Meta().res_passthrough,
      FilterType::Meta().res_max,
      // Can't use std::max(0 - 1, 0) on unsigneds...
      FilterType::Meta().output_delay > 0
        ? FilterType::Meta().output_delay - 1 : 0,
      FilterType::Meta().output_gain);
    return metas;
  }

 private:
  FilterType filter_;
};

}  // namespace filters
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_FILTERS_OVERSAMPLER_H_
