/// @file common.h
/// @brief SoundTailor common utilities header
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

#ifndef SOUNDTAILOR_SRC_COMMON_H_
#define SOUNDTAILOR_SRC_COMMON_H_

#include <cassert>

#include "soundtailor/src/configuration.h"

#if (_USE_SSE)
extern "C" {
  // @todo(gm) move that include
#include <emmintrin.h>
#include <mmintrin.h>
}
#endif  // (_USE_SSE)

namespace soundtailor {

/// @brief Ignore unused variables
template<typename Type> void IGNORE(const Type&) {}

/// @brief Assume that the following condition is always true
/// (on some compilers, allows optimization)
#if(_COMPILER_MSVC)
  static inline void ASSUME(const bool condition) {_assume(condition);}
#elif(_COMPILER_GCC)
  static inline void ASSUME(const bool condition) {if (!(condition)) __builtin_unreachable();}
#else
  #define ASSUME(_condition_)
#endif  // _COMPILER_ ?

/// @brief Asserts condition == true
#if(_BUILD_CONFIGURATION_DEBUG)
  #define SOUNDTAILOR_ASSERT(_condition_) (assert((_condition_)))
#else
  // Maps to "assume" in release configuration for better optimization
  #define SOUNDTAILOR_ASSERT(_condition_) {::soundtailor::ASSUME((_condition_));}
#endif

/// @brief "Sample" type - actually, this is the data computed at each "tick";
/// If using vectorization it will be longer than 1 audio sample
#if (_USE_SSE)
  typedef __m128 Sample;
#else
  struct Sample { float data_[4]; };
#endif  // (_USE_SSE)

/// @brief Type for Sample parameter "read only":
/// It should be passed by value since it allows to keep it into a register,
/// instead of passing its address and loading it.
typedef const Sample SampleRead;

/// @brief Type for block input parameter
typedef const float* const BlockIn;

/// @brief Type for block output parameter
typedef float* const BlockOut;

/// @brief "Sample" type size in bytes
static const unsigned int SampleSizeBytes(sizeof(Sample));
/// @brief "Sample" type size compared to audio samples
static const unsigned int SampleSize(sizeof(Sample) / sizeof(float));

}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_COMMON_H_
