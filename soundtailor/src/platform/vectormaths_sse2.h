/// @file maths_sse2.h
/// @brief SoundTailor maths header - SSE2 implementation
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

#ifndef SOUNDTAILOR_SRC_PLATFORM_VECTORMATHS_SSE2_H_
#define SOUNDTAILOR_SRC_PLATFORM_VECTORMATHS_SSE2_H_

#include <cmath>
// std::min, std::max
#include <algorithm>

#include "soundtailor/src/common.h"

#if _USE_SSE

extern "C" {
#include <emmintrin.h>
#include <mmintrin.h>
}

namespace soundtailor {
struct SSE2VectorMath {
  /// @brief Fill a whole Sample with the given value
  ///
  /// @param[in]  value   Value to be copied through the whole Sample
  static inline Sample Fill(const float value) {
    return _mm_set1_ps(value);
  }

  /// @brief Fill a whole Sample with the given float array
  ///
  /// @param[in]  value   Pointer to the float array to be used:
  ///                     must be SampleSizeBytes long
  static inline Sample Fill(const float* value) {
    return _mm_load_ps(value);
  }

  /// @brief Fill a whole Sample with all given scalars,
  /// beware of the order: SSE is "little-endian" (sort of)
  ///
  /// @param[in]  a   Last value
  /// @param[in]  b   Second to last value
  /// @param[in]  c   Second value
  /// @param[in]  d   First value
  static inline Sample Fill(const float a,
                     const float b,
                     const float c,
                     const float d) {
    return _mm_set_ps(d, c, b, a);
  }

  /// @brief Helper union for vectorized type to scalar array conversion
  typedef union {
    Sample sample_v;  ///< Vectorized type
    float sample[soundtailor::SampleSize];  ///< Array of scalars
  } ConverterFloatScalarVector;
  /// @brief Helper union for vectorized integer type to scalar array conversion
  typedef union {
    IntVec sample_v;  ///< Vectorized type
    int sample[soundtailor::SampleSize];  ///< Array of scalars
  } ConverterIntScalarVector;

  /// @brief Extract one element from a Sample (compile-time version)
  ///
  /// @param[in]  input   Sample to be read
  // TODO(gm): faster _mm_store_ss specialization
  template<unsigned i>
  static float GetByIndex(SampleRead input) {
    ConverterFloatScalarVector converter;
    converter.sample_v = input;
    return converter.sample[i];
  }

  /// @brief Integer version of the above
  template<unsigned i>
  static int GetByIndex(IntVec input) {
    ConverterIntScalarVector converter;
    converter.sample_v = input;
    return converter.sample[i];
  }

  /// @brief Extract one element from a Sample (runtime version, in loops)
  ///
  /// @param[in]  input   Sample to be read
  /// @param[in]  i   Index of the element to retrieve
  static inline float GetByIndex(SampleRead input, const unsigned i) {
    SOUNDTAILOR_ASSERT(i < soundtailor::SampleSize);
    ConverterFloatScalarVector converter;
    converter.sample_v = input;
    return converter.sample[i];
  }

  /// @brief Add "left" to "right"
  static inline Sample Add(SampleRead left, SampleRead right) {
    return _mm_add_ps(left, right);
  }

  /// @brief Sum all elements of a Sample
  static inline float AddHorizontal(SampleRead input) {
    const Sample first_add(Add(input, _mm_movehl_ps(input, input)));
    const Sample shuffled(_mm_shuffle_ps(first_add, first_add,
                          _MM_SHUFFLE(0, 1, 0, 1)));
    return GetByIndex<0>(Add(shuffled, first_add));
  }

  /// @brief Substract "right" from "left"
  static inline Sample Sub(SampleRead left, SampleRead right) {
    return _mm_sub_ps(left, right);
  }

  /// @brief Element-wise multiplication
  static inline Sample Mul(SampleRead left, SampleRead right) {
    return _mm_mul_ps(left, right);
  }

  /// @brief Shift to right all elements of the input by 1,
  /// and shift in the given value
  ///
  /// E.g. given (x_{n}, x_{n + 1}, x_{n + 2}, x_{n + 3})
  /// return (value, x_{n}, x_{n + 1}, x_{n + 2})
  ///
  /// @param[in]  input   Sample to be shifted
  /// @param[in]  value   value to be shifted in
  static inline Sample RotateOnRight(SampleRead input,
                              const float value) {
    // beware of the order: SSE is "little-endian" (sort of)
    const Sample rotated(_mm_castsi128_ps(
      _mm_slli_si128(_mm_castps_si128(input), 4)));
    return Add(Fill(value, 0.0f, 0.0f, 0.0f), rotated);
  }

  /// @brief Shift to left all elements of the input by 1,
  /// and shift in the given value
  ///
  /// E.g. given (x_{n}, x_{n + 1}, x_{n + 2}, x_{n + 3})
  /// return (x_{n + 1}, x_{n + 2}, x_{n + 3}, value)
  ///
  /// @param[in]  input   Sample to be shifted
  /// @param[in]  value   value to be shifted in
  static inline Sample RotateOnLeft(SampleRead input,
                             const float value) {
    // beware of the order: SSE is "little-endian" (sort of)
    const Sample rotated(_mm_castsi128_ps(
      _mm_srli_si128(_mm_castps_si128(input), 4)));
    return Add(Fill(0.0f, 0.0f, 0.0f, value), rotated);
  }

  /// @brief Return the sign of each element of the Sample
  ///
  /// Sgn(0.0) return 0.0
  static inline Sample Sgn(SampleRead input) {
    const Sample kZero(_mm_setzero_ps());
    const Sample kOne(Fill(1.0f));
    const Sample kMinus(Fill(-1.0f));
    const Sample kPlusMask(_mm_and_ps(_mm_cmpgt_ps(input, kZero), kOne));
    const Sample kMinusMask(_mm_and_ps(_mm_cmplt_ps(input, kZero), kMinus));
    return Add(kPlusMask, kMinusMask);
  }

  /// @brief Return the sign of each element of the Sample, no zero version
  ///
  /// Sgn(0.0) return 1.0f
  static inline Sample SgnNoZero(SampleRead value) {
    const Sample kZero(_mm_setzero_ps());
    const Sample kOne(Fill(1.0f));
    const Sample kMinus(Fill(-1.0f));
    const Sample kPlusMask(_mm_and_ps(_mm_cmpge_ps(value, kZero), kOne));
    const Sample kMinusMask(_mm_and_ps(_mm_cmplt_ps(value, kZero), kMinus));
    return Add(kPlusMask, kMinusMask);
  }

  /// @brief Store the given Sample into memory
  ///
  /// @param[in]  buffer   Memory to be filled with the input
  /// @param[in]  input   Sampel to be stored
  static inline void Store(float* const buffer, SampleRead input) {
    _mm_store_ps(buffer, input);
  }

  static inline void StoreUnaligned(float* const buffer, SampleRead input) {
    _mm_storeu_ps(buffer, input);
  }

  /// @brief Get each right half of the two given vectors
  ///
  /// Given left = (x0, x1, x2, x3) and right = (y0, y1, y2, y3)
  /// it will return (x2, x3, y2, y3)
  static inline Sample TakeEachRightHalf(SampleRead left,
                                         SampleRead right) {
    // beware of the order: SSE is "little-endian" (sort of)
    return _mm_shuffle_ps(left, right, _MM_SHUFFLE(3, 2, 3, 2));
  }

  /// @brief Revert the given vector values order
  ///
  /// Given value = (x0, x1, x2, x3)
  /// it will return (x3, x2, x1, x0)
  static inline Sample Revert(SampleRead value) {
    return _mm_shuffle_ps(value, value, _MM_SHUFFLE(0, 1, 2, 3));
  }

  /// @brief Return each min element of both inputs
  static inline Sample Min(SampleRead left, SampleRead right) {
    return _mm_min_ps(left, right);
  }

  /// @brief Return each max element of both inputs
  static inline Sample Max(SampleRead left, SampleRead right) {
#if (_USE_SSE)
    return _mm_max_ps(left, right);
#else
    return std::max(left, right);
#endif  // (_USE_SSE)
  }

  /// @brief Round each Sample element to the nearest integer
  static inline Sample Round(SampleRead input) {
#if (_USE_SSE)
    const Sample kZero(_mm_setzero_ps());
    const Sample kPlus(Fill(0.5f));
    const Sample kMinus(Fill(-0.5f));
    const Sample kPlusMask(_mm_and_ps(_mm_cmpge_ps(input, kZero), kPlus));
    const Sample kMinusMask(_mm_and_ps(_mm_cmplt_ps(input, kZero), kMinus));
    const Sample kAddMask(Add(kPlusMask, kMinusMask));
    return Add(kAddMask, input);
#else
    return (input > 0.0f) ? (input + 0.5f) : (input - 0.5f);
#endif  // (_USE_SSE)
  }

  /// @brief Helper function: increment the input and wraps it into [-1.0 ; 1.0[
  ///
  /// @param[in]  input         Input to be wrapped - supposed not to be < 1.0
  /// @param[in]  increment     Increment to add to the input
  /// @return the incremented output in [-1.0 ; 1.0[
  static inline Sample IncrementAndWrap(SampleRead input, SampleRead increment) {
    // TODO(gm): check if a common code path can be found
#if (_USE_SSE)
    const Sample output(Add(input, increment));
    const Sample constant(Fill(-2.0f));
    const Sample threshold(Fill(1.0f));
    const Sample addition_mask(_mm_cmpgt_ps(output, threshold));
    const Sample add(_mm_and_ps(addition_mask, constant));
    return Add(output, add);
#else
    Sample output(Add(input, increment));
    if (output > 1.0f) {
      const float constant(-2.0f);
      output = Add(output, constant);
    }
    return output;
#endif  // (_USE_SSE)
  }

  /// @brief Helper binary function: return true if all input elements are true
  ///
  /// @param[in]  input   Input to be checked
  static inline bool IsMaskFull(SampleRead input) {
    return 15 == _mm_movemask_ps(input);
  }

  /// @brief Helper binary function: return true if all input elements are null
  ///
  /// @param[in]  input   Input to be checked
  static inline bool IsMaskNull(SampleRead input) {
    return 0 == _mm_movemask_ps(input);
  }

  static inline Sample GreaterEqual(SampleRead threshold, SampleRead input) {
    return _mm_cmpge_ps(threshold, input);
  }

  static inline Sample GreaterThan(SampleRead threshold, SampleRead input) {
    return _mm_cmpgt_ps(threshold, input);
  }

  /// @brief Helper binary function:
  /// true if each threshold element is >= than the input element
  static inline bool GreaterEqual(float threshold, SampleRead input) {
    const Sample test_result(GreaterEqual(Fill(threshold), input));
    return IsMaskFull(test_result);
  }

  static inline bool GreaterEqualAny(float threshold, SampleRead input) {
    const Sample test_result(GreaterEqual(Fill(threshold), input));
    return !IsMaskNull(test_result);
  }

  static inline bool GreaterThan(float threshold, SampleRead input) {
    const Sample test_result(GreaterThan(Fill(threshold), input));
    return IsMaskFull(test_result);
  }

  static inline Sample LessEqual(SampleRead threshold, SampleRead input) {
    return _mm_cmple_ps(threshold, input);
  }

  static inline Sample LessThan(SampleRead threshold, SampleRead input) {
    return _mm_cmplt_ps(threshold, input);
  }

  /// @brief Helper binary function:
  /// true if each threshold element is <= than the input element
  static inline bool LessEqual(float threshold, SampleRead input) {
    const Sample test_result(LessEqual(Fill(threshold), input));
    return IsMaskFull(test_result);
  }

  static inline bool LessThan(float threshold, SampleRead input) {
    const Sample test_result(LessThan(Fill(threshold), input));
    return IsMaskFull(test_result);
  }

  static inline Sample Equal(SampleRead threshold, SampleRead value) {
    return _mm_cmpeq_ps(threshold, value);
  }

  static inline bool Equal(float threshold, SampleRead input) {
    const Sample test_result(Equal(Fill(threshold), input));
    return IsMaskFull(test_result);
  }

  /// @brief Beware, not an actual bitwise AND! More like a "float select"
  static inline Sample ExtractValueFromMask(SampleRead value, SampleRead mask) {
    return _mm_and_ps(left, right);
  }

  static inline IntVec TruncToInt(SampleRead float_value) {
    return _mm_cvttps_epi32(float_value);
  }
};

typedef SSE2VectorMath PlatformVectorMath;
}  // namespace soundtailor

#endif  // _USE_SSE

#endif  // SOUNDTAILOR_SRC_PLATFORM_VECTORMATHS_SSE2_H_
