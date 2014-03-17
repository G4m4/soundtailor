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

namespace soundtailor {

/// @brief Ignore unused variables
template<typename Type> void IGNORE(const Type&) {}

/// @brief Assume that the following condition is always true
/// (on some compilers, allows optimization)
#if(_COMPILER_MSVC)
  #define ASSUME(_condition_) _assume((_condition_));
#elif(_COMPILER_GCC)
  #define ASSUME(_condition_) if (!(_condition_)) __builtin_unreachable();
#else
  #define ASSUME(_condition_)
#endif  // _COMPILER_ ?

/// @brief Asserts _condition_ == true
#if(_BUILD_CONFIGURATION_DEBUG)
  #define ASSERT(_condition_) assert(_condition_)
#else
  // Maps to "assume" in release configuration for better optimization
  #define ASSERT(_condition_) ASSUME(_condition_)
#endif

/// @brief Attribute for structures alignment
#if (_USE_SSE)
  #if (_COMPILER_MSVC)
    #define ALIGN __declspec(align(16))
  #else
    #define ALIGN __attribute__((aligned(16)))
  #endif
#else
  #define ALIGN
#endif  // (_USE_SSE)

}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_COMMON_H_
