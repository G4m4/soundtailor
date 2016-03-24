/// @file maths_std.h
/// @brief SoundTailor maths header - standard implementation
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

#ifndef SOUNDTAILOR_SRC_PLATFORM_MATHS_STD_H_
#define SOUNDTAILOR_SRC_PLATFORM_MATHS_STD_H_

#include <cmath>
// std::min, std::max
#include <algorithm>

#include "soundtailor/src/common.h"

namespace soundtailor {
struct StandardMath {
  static inline float Abs(const float value) {
    return std::abs(value);
  }

  /// @brief Round the given number down to the greatest smaller multiple of 4
  /// e.g. 13 => 12, 12 => 12, 11 => 8
  static inline unsigned int GetMultipleOf4(unsigned int value) {
    return value - (value % 4);
  }
};

typedef StandardMath PlatformMath;
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_PLATFORM_MATHS_STD_H_
