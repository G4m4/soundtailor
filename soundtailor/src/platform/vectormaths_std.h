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

#ifndef SOUNDTAILOR_SRC_PLATFORM_VECTORMATHS_STD_H_
#define SOUNDTAILOR_SRC_PLATFORM_VECTORMATHS_STD_H_

#if _USE_SSE == 0

#include <cmath>
// std::memcpy
#include <cstring>
// std::min, std::max
#include <algorithm>

#include "soundtailor/src/common.h"

namespace soundtailor {
struct StandardVectorMath {
  /// @brief Fill a whole Sample with all given scalars
  ///
  /// @param[in]  a   Last value
  /// @param[in]  b   Second to last value
  /// @param[in]  c   Second value
  /// @param[in]  d   First value
  static inline Sample Fill(const float a,
                            const float b,
                            const float c,
                            const float d) {
    return {{ a, b, c, d }};
  }

  /// @brief Fill a whole Sample with the given value
  ///
  /// @param[in]  value   Value to be copied through the whole Sample
  static inline Sample Fill(const float value) {
    return Fill( value, value, value, value );
  }

  /// @brief Fill a whole Sample with the given float array
  /// beware of the order: SSE is "little-endian" (sort of)
  ///
  /// @param[in]  value   Pointer to the float array to be used:
  ///                     must be SampleSizeBytes long
  static inline Sample Fill(const float* value) {
    return Fill( value[0], value[1], value[2], value[3] );
  }

  /// @brief Extract one element from a Sample (compile-time version)
  ///
  /// @param[in]  input   Sample to be read
  // TODO(gm): faster _mm_store_ss specialization
  template<unsigned i>
  static float GetByIndex(SampleRead input) {
    return input.data_[i];
  }

  /// @brief Extract one element from a Sample (runtime version, in loops)
  ///
  /// @param[in]  input   Sample to be read
  /// @param[in]  i   Index of the element to retrieve
  static inline float GetByIndex(SampleRead input, const unsigned i) {
    SOUNDTAILOR_ASSERT(i < SampleSize);
    return input.data_[i];
  }

  /// @brief Add "left" to "right"
  static inline Sample Add(SampleRead left, SampleRead right) {
    return Fill(
      left.data_[0] + right.data_[0],
      left.data_[1] + right.data_[1],
      left.data_[2] + right.data_[2],
      left.data_[3] + right.data_[3] );
  }

  /// @brief Sum all elements of a Sample
  static inline float AddHorizontal(SampleRead input) {
    return input.data_[0]
      + input.data_[1]
      + input.data_[2]
      + input.data_[3];
  }

  /// @brief Substract "right" from "left"
  static inline Sample Sub(SampleRead left, SampleRead right) {
    return Fill(
      left.data_[0] - right.data_[0],
      left.data_[1] - right.data_[1],
      left.data_[2] - right.data_[2],
      left.data_[3] - right.data_[3] );
  }

  /// @brief Element-wise multiplication
  static inline Sample Mul(SampleRead left, SampleRead right) {
    return Fill(
      left.data_[0] * right.data_[0],
      left.data_[1] * right.data_[1],
      left.data_[2] * right.data_[2],
      left.data_[3] * right.data_[3] );
  }

  /// @brief Shift to right all elements of the input by 1,
  /// and shift in the given value
  ///
  /// E.g. given (x_{n}, x_{n + 1}, x_{n + 2}, x_{n + 3})
  /// return (value, x_{n}, x_{n + 1}, x_{n + 2})
  ///
  /// @param[in]  input   Sample to be shifted
  /// @param[in]  value   value to be shifted in
  static inline Sample RotateOnRight(SampleRead input, const float value) {
    return Fill(
      value,
      input.data_[0],
      input.data_[1],
      input.data_[2]);
  }

  /// @brief Shift to left all elements of the input by 1,
  /// and shift in the given value
  ///
  /// E.g. given (x_{n}, x_{n + 1}, x_{n + 2}, x_{n + 3})
  /// return (x_{n + 1}, x_{n + 2}, x_{n + 3}, value)
  ///
  /// @param[in]  input   Sample to be shifted
  /// @param[in]  value   value to be shifted in
  static inline Sample RotateOnLeft(SampleRead input, const float value) {
    return Fill(
      input.data_[1],
      input.data_[2],
      input.data_[3],
      value);
  }

  /// @brief Return the sign of each element of the Sample
  ///
  /// Sgn(0.0) return 0.0
  static inline Sample Sgn(SampleRead input) {
    return Fill(
      static_cast<float>(input.data_[0] > 0.0f) - static_cast<float>(input.data_[0] < 0.0f),
      static_cast<float>(input.data_[1] > 0.0f) - static_cast<float>(input.data_[1] < 0.0f),
      static_cast<float>(input.data_[2] > 0.0f) - static_cast<float>(input.data_[2] < 0.0f),
      static_cast<float>(input.data_[3] > 0.0f) - static_cast<float>(input.data_[3] < 0.0f) );
  }

  /// @brief Return the sign of each element of the Sample, no zero version
  ///
  /// Sgn(0.0) return 1.0f
  static inline Sample SgnNoZero(SampleRead input) {
    return Fill(
      input.data_[0] >= 0.0f ? 1.0f : -1.0f,
      input.data_[1] >= 0.0f ? 1.0f : -1.0f,
      input.data_[2] >= 0.0f ? 1.0f : -1.0f,
      input.data_[3] >= 0.0f ? 1.0f : -1.0f );
  }

  /// @brief Store the given Sample into memory
  ///
  /// @param[in]  buffer   Memory to be filled with the input
  /// @param[in]  input   Sample to be stored
  static inline void Store(float* const buffer, SampleRead input) {
    std::memcpy(buffer, &input.data_[0], sizeof(input));
  }

  static inline void StoreUnaligned(float* const buffer, SampleRead input) {
    Store(buffer, input);
  }

  /// @brief Get each right half of the two given vectors
  ///
  /// Given left = (x0, x1, x2, x3) and right = (y0, y1, y2, y3)
  /// it will return (x2, x3, y2, y3)
  static inline Sample TakeEachRightHalf(SampleRead left,
                                         SampleRead right) {
    return Fill( left.data_[2], left.data_[3],
             right.data_[2], right.data_[3] );
  }

  /// @brief Revert the given vector values order
  ///
  /// Given value = (x0, x1, x2, x3)
  /// it will return (x3, x2, x1, x0)
  static inline Sample Revert(SampleRead input) {
    return Fill(
      input.data_[3],
      input.data_[2],
      input.data_[1],
      input.data_[0] );
  }

  /// @brief Return each min element of both inputs
  static inline Sample Min(SampleRead left, SampleRead right) {
    return Fill(
      left.data_[0] < right.data_[0] ? left.data_[0] : right.data_[0],
      left.data_[1] < right.data_[1] ? left.data_[1] : right.data_[1],
      left.data_[2] < right.data_[2] ? left.data_[2] : right.data_[2],
      left.data_[3] < right.data_[3] ? left.data_[3] : right.data_[3] );
  }

  /// @brief Return each max element of both inputs
  static inline Sample Max(SampleRead left, SampleRead right) {
    return Fill(
      left.data_[0] > right.data_[0] ? left.data_[0] : right.data_[0],
      left.data_[1] > right.data_[1] ? left.data_[1] : right.data_[1],
      left.data_[2] > right.data_[2] ? left.data_[2] : right.data_[2],
      left.data_[3] > right.data_[3] ? left.data_[3] : right.data_[3] );
  }

  /// @brief Round each Sample element to the nearest integer
  static inline Sample Round(SampleRead input) {
    return Fill(
      input.data_[0] > 0.0f ? input.data_[0] + 0.5f : input.data_[0] - 0.5f,
      input.data_[1] > 0.0f ? input.data_[1] + 0.5f : input.data_[1] - 0.5f,
      input.data_[2] > 0.0f ? input.data_[2] + 0.5f : input.data_[2] - 0.5f,
      input.data_[3] > 0.0f ? input.data_[3] + 0.5f : input.data_[3] - 0.5f );
  }
  /// @brief Helper function: increment the input and wraps it into [-1.0 ; 1.0[
  ///
  /// @param[in]  input         Input to be wrapped - supposed not to be < 1.0
  /// @param[in]  increment     Increment to add to the input
  /// @return the incremented output in [-1.0 ; 1.0[
  static inline Sample IncrementAndWrap(SampleRead input, SampleRead increment) {
    const Sample output(Add(input, increment));
    const Sample additional_increment(Fill(
      output.data_[0] > 1.0f ? -2.0f : 0.0f,
      output.data_[1] > 1.0f ? -2.0f : 0.0f,
      output.data_[2] > 1.0f ? -2.0f : 0.0f,
      output.data_[3] > 1.0f ? -2.0f : 0.0f ));

    return Add(output, additional_increment);
  }

  /// @brief Helper binary function: return true if all input elements are true
  ///
  /// @param[in]  input   Input to be checked
  static inline bool IsMaskFull(SampleRead input) {
    return input.data_[0] > 0.0f
        && input.data_[1] > 0.0f
        && input.data_[2] > 0.0f
        && input.data_[3] > 0.0f;
  }

  /// @brief Helper binary function: return true if all input elements are null
  ///
  /// @param[in]  input   Input to be checked
  static inline bool IsMaskNull(SampleRead input) {
    return input.data_[0] == 0.0f
      && input.data_[1] == 0.0f
      && input.data_[2] == 0.0f
      && input.data_[3] == 0.0f;
  }

  /// @brief Helper binary function:
  /// true if each threshold element is >= than the matching input element
  static inline Sample GreaterEqual(SampleRead threshold, SampleRead input) {
    return Fill(
      threshold.data_[0] >= input.data_[0] ? 0xffffffff : 0.0f,
      threshold.data_[1] >= input.data_[1] ? 0xffffffff : 0.0f,
      threshold.data_[2] >= input.data_[2] ? 0xffffffff : 0.0f,
      threshold.data_[3] >= input.data_[3] ? 0xffffffff : 0.0f );
  }

  static inline Sample GreaterThan(SampleRead threshold, SampleRead input) {
    return Fill(
      threshold.data_[0] > input.data_[0] ? 0xffffffff : 0.0f,
      threshold.data_[1] > input.data_[1] ? 0xffffffff : 0.0f,
      threshold.data_[2] > input.data_[2] ? 0xffffffff : 0.0f,
      threshold.data_[3] > input.data_[3] ? 0xffffffff : 0.0f );
  }

  /// @brief Helper binary function:
  /// true if any input element is >= than any of the threshold element
  static inline bool GreaterEqual(float threshold, SampleRead input) {
    return threshold >= input.data_[0]
      && threshold >= input.data_[1]
      && threshold >= input.data_[2]
      && threshold >= input.data_[3];
  }

  static inline bool GreaterEqualAny(float threshold, SampleRead input) {
    return threshold >= input.data_[0]
      || threshold >= input.data_[1]
      || threshold >= input.data_[2]
      || threshold >= input.data_[3];
  }

  static inline bool GreaterThan(float threshold, SampleRead input) {
    return threshold > input.data_[0]
      && threshold > input.data_[1]
      && threshold > input.data_[2]
      && threshold > input.data_[3];
  }

  /// @brief Helper binary function:
  /// true if each threshold element is <= than the input element
  static inline Sample LessEqual(SampleRead threshold, SampleRead input) {
    return Fill(
      threshold.data_[0] <= input.data_[0] ? 0xffffffff : 0.0f,
      threshold.data_[1] <= input.data_[1] ? 0xffffffff : 0.0f,
      threshold.data_[2] <= input.data_[2] ? 0xffffffff : 0.0f,
      threshold.data_[3] <= input.data_[3] ? 0xffffffff : 0.0f );
  }

  static inline Sample LessThan(SampleRead threshold, SampleRead input) {
    return Fill(
      threshold.data_[0] < input.data_[0] ? 0xffffffff : 0.0f,
      threshold.data_[1] < input.data_[1] ? 0xffffffff : 0.0f,
      threshold.data_[2] < input.data_[2] ? 0xffffffff : 0.0f,
      threshold.data_[3] < input.data_[3] ? 0xffffffff : 0.0f );
  }

  static inline bool LessEqual(float threshold, SampleRead input) {
    return threshold <= input.data_[0]
      && threshold <= input.data_[1]
      && threshold <= input.data_[2]
      && threshold <= input.data_[3];
  }

  static inline bool LessThan(float threshold, SampleRead input) {
    return threshold < input.data_[0]
      && threshold < input.data_[1]
      && threshold < input.data_[2]
      && threshold < input.data_[3];
  }

  static inline Sample Equal(SampleRead threshold, SampleRead input) {
    return Fill(
      threshold.data_[0] == input.data_[0] ? 0xffffffff : 0.0f,
      threshold.data_[1] == input.data_[1] ? 0xffffffff : 0.0f,
      threshold.data_[2] == input.data_[2] ? 0xffffffff : 0.0f,
      threshold.data_[3] == input.data_[3] ? 0xffffffff : 0.0f );
  }

  static inline bool Equal(float threshold, SampleRead input) {
    return threshold == input.data_[0]
      && threshold == input.data_[1]
      && threshold == input.data_[2]
      && threshold == input.data_[3];
  }
};

typedef StandardVectorMath PlatformVectorMath;
}  // namespace soundtailor

#endif  // _USE_SSE == 0

#endif  // SOUNDTAILOR_SRC_PLATFORM_VECTORMATHS_STD_H_
