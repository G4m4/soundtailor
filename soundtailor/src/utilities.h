/// @file utilities.h
/// @brief SoundTailor common maths header
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

#ifndef SOUNDTAILOR_SRC_UTILITIES_H_
#define SOUNDTAILOR_SRC_UTILITIES_H_

#include "soundtailor/src/maths.h"

namespace soundtailor {

/// @brief Block process function (generators-like)
///
/// In a context of dynamic polymorphism this will save you from per-sample
/// virtual function calls
/// The compiler should be able to inline it
/// obviously the instance has to be known at compile time
template <typename GeneratorType>
void ProcessBlock(BlockOut out,
                  std::size_t block_size,
                  GeneratorType&& instance) {
  float* SOUNDTAILOR_RESTRICT out_write(out);
  for (std::size_t i(0); i < block_size; i += SampleSize) {
    VectorMath::Store(out_write, instance());
    out_write += SampleSize;
  }
}

/// @brief Block process function (filters-like)
///
/// In a context of dynamic polymorphism this will save you from per-sample
/// virtual function calls
/// The compiler should be able to inline it
/// obviously the instance has to be known at compile time
template <typename FilterType>
void ProcessBlock(BlockIn in,
                  BlockOut out,
                  std::size_t block_size,
                  FilterType&& filter_instance) {
  const float* SOUNDTAILOR_RESTRICT in_ptr(in);
  float* SOUNDTAILOR_RESTRICT out_write(out);
  for (std::size_t i(0); i < block_size; i += SampleSize) {
    const Sample kInput(VectorMath::Fill(in_ptr));
    VectorMath::Store(out_write, filter_instance(kInput));
    in_ptr += SampleSize;
    out_write += SampleSize;
  }
}

}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_UTILITIES_H_
