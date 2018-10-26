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
#if(_SOUNDTAILOR_COMPILER_MSVC)
  static inline void ASSUME(const bool condition) {__assume(condition);}
#elif(_SOUNDTAILOR_COMPILER_GCC)
  static inline void ASSUME(const bool condition) {if (!(condition)) __builtin_unreachable();}
#else
  #error Unknown compiler!
  #define ASSUME(_condition_)
#endif  // _SOUNDTAILOR_COMPILER_ ?

/// @brief Asserts condition == true
#if(_SOUNDTAILOR_BUILD_CONFIGURATION_DEBUG)
  #define SOUNDTAILOR_ASSERT(_condition_) (assert((_condition_)))
#else
  // Maps to "assume" in release configuration for better optimization
  #define SOUNDTAILOR_ASSERT(_condition_) {::soundtailor::ASSUME((_condition_));}
#endif

/// @brief Indicates that the decorated reference is not aliased
#ifndef SOUNDTAILOR_RESTRICT
  #if(_SOUNDTAILOR_COMPILER_MSVC)
    #define SOUNDTAILOR_RESTRICT __restrict
  #elif(_SOUNDTAILOR_COMPILER_GCC)
    #define SOUNDTAILOR_RESTRICT __restrict__
  #else
    #define SOUNDTAILOR_RESTRICT
  #endif  // _SOUNDTAILOR_COMPILER_ ?
#endif  // SOUNDTAILOR_RESTRICT ?

}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_COMMON_H_
