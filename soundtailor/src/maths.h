/// @file maths.h
/// @brief SoundTailor common maths header
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

#ifndef SOUNDTAILOR_SRC_MATHS_H_
#define SOUNDTAILOR_SRC_MATHS_H_

#include <cmath>
#include <cstddef> // size_t
// std::min, std::max
#include <algorithm>

#include "vecmath/inc/maths.h"

#include "soundtailor/src/common.h"

namespace soundtailor {

/// @brief Standard value for Pi
const double Pi = 3.14159265358979;

typedef vecmath::PlatformVectorMath::FloatVec Sample;
typedef vecmath::PlatformVectorMath::IntVec SampleInt;
typedef vecmath::PlatformVectorMath::FloatVecRead SampleRead;
/// @brief "Sample" type size in bytes
static const unsigned int SampleSizeBytes(sizeof(Sample));
/// @brief "Sample" type size compared to audio samples
static const unsigned int SampleSize(sizeof(Sample) / sizeof(float));

/// @brief Type for block input parameter
typedef const float* SOUNDTAILOR_RESTRICT const BlockIn;

/// @brief Type for block output parameter
typedef float* SOUNDTAILOR_RESTRICT const BlockOut;

struct VectorMath : vecmath::PlatformVectorMath {

  /// @brief Fill a whole Sample with the given (scalar) generator
  ///
  /// @param[in]  generator   Generator to fill the Sample with
  template <typename TypeGenerator>
  static inline Sample FillWithFloatGenerator(TypeGenerator& generator) {
    const float a(generator());
    const float b(generator());
    const float c(generator());
    const float d(generator());
    return Fill(a,
                b,
                c,
                d);
  }

  /// @brief Fill a whole Sample with incremental values as follows:
  ///
  /// First value:  base
  /// ...
  /// Last value:  base + SampleSize * increment
  ///
  /// @param[in]  base    Base value to fill the first element of the Sample with
  /// @param[in]  increment    Value to add at each Sample element
  static inline Sample FillIncremental(const float base,
                                       const float increment) {
    return Fill(base,
                base + increment,
                base + increment * 2.0f,
                base + increment * 3.0f);
  }

  /// @brief Fill a whole Sample based on its length
  ///
  /// The actual value is SampleSize * given value
  ///
  /// @param[in]  base    Base value to be filled with
  static inline Sample FillOnLength(const float base) {
    return Fill(base * soundtailor::SampleSize);
  }

  /// @brief Extract first element from a Sample
  ///
  /// @param[in]  input   Sample to be read
  static inline float GetFirst(SampleRead input) {
    return GetByIndex<0>(input);
  }

  /// @brief Extract last element from a Sample
  ///
  /// @param[in]  input   Sample to be read
  static inline float GetLast(SampleRead input) {
    return GetByIndex<3>(input);
  }

  /// @brief Helper function: limit input into [min ; max]
  static inline Sample Clamp(SampleRead input,
                             const SampleRead min,
                             const SampleRead max) {
    return Min(Max(input, min), max);
  }

  /// @brief Multiply a Sample by a scalar constant
  ///
  /// @param[in]  constant   Scalar constant to multiply the Sample by
  /// @param[in]  input   Sample to be multiplied
  static inline Sample MulConst(const float constant, SampleRead input) {
    return Mul(Fill(constant), input);
  }

  /// @brief Normalize the input based on actual Sample length
  ///
  /// @param[in]  input    Value to be normalized
  static inline Sample Normalize(SampleRead input) {
    // Note: division deliberately avoided
    return MulConst(0.25f, input);
  }

  /// @brief Return the absolute value of each element of the Sample
  static inline Sample Abs(SampleRead input) {
    return Max(Sub(Fill(0.0f), input), input);
  }

  static inline bool Equal(float threshold, SampleRead input) {
    const Sample test_result(vecmath::PlatformVectorMath::Equal(Fill(threshold), input));
    return IsMaskFull(test_result);
  }

  static inline bool Equal(SampleRead threshold, SampleRead input) {
    const Sample test_result(vecmath::PlatformVectorMath::Equal(threshold, input));
    return IsMaskFull(test_result);
  }

  /// @brief Helper binary function:
  /// true if both input are closer than the given threshold
  ///
  /// @param[in]  left   First Sample
  /// @param[in]  right   Second Sample
  /// @param[in]  threshold   Threshold below which samples are considered close
  static inline bool IsNear(SampleRead left,
                            SampleRead right,
                            const float threshold) {
    const Sample abs_diff(Abs(Sub(left, right)));
    return GreaterEqual(threshold, abs_diff);
  }

  /// @brief Helper binary function:
  /// true if any input elements are closer than the given threshold
  ///
  /// @param[in]  left   First Sample
  /// @param[in]  right   Second Sample
  /// @param[in]  threshold   Threshold below which samples are considered close
  static inline bool IsAnyNear(SampleRead left,
                               SampleRead right,
                               const float threshold) {
    const Sample abs_diff(Abs(Sub(left, right)));
    return GreaterEqualAny(threshold, abs_diff);
  }
};

}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_MATHS_H_
