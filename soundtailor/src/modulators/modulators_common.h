/// @file modulators_common.h
/// @brief SoundTailor common stuff for all modulators
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

#ifndef SOUNDTAILOR_SRC_MODULATORS_MODULATORS_COMMON_H_
#define SOUNDTAILOR_SRC_MODULATORS_MODULATORS_COMMON_H_

namespace soundtailor {
namespace modulators {

/// @brief Available parts of an envelop
enum Section {
  kAttack = 0,
  kDecay,
  kSustain,
  kRelease,
  kZero
};

/// @brief Get the section after the given one
Section GetNextSection(const Section enum_value);

}  // namespace modulators
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_MODULATORS_MODULATORS_COMMON_H_
