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
// std::min, std::max
#include <algorithm>

#include "soundtailor/src/common.h"

namespace soundtailor {

/// @brief Standard value for Pi
static const double Pi(3.14159265358979);

/// @brief Fill a whole Sample with the given value
///
/// @param[in]  value   Value to be copied through the whole Sample
static inline Sample Fill(const float value) {
#if (_USE_SSE)
  return _mm_set1_ps(value);
#else
  return value;
#endif  // (_USE_SSE)
}

/// @brief Fill a whole Sample with the given float array
///
/// @param[in]  value   Pointer to the float array to be used:
///                     must be SampleSizeBytes long
static inline Sample Fill(const float* value) {
#if (_USE_SSE)
  return _mm_loadu_ps(value);
#else
  return *value;
#endif  // (_USE_SSE)
}

/// @brief Fill a whole Sample with all given scalars,
/// beware of the order!
///
/// @param[in]  a   Last value
/// @param[in]  b   Second to last value
/// @param[in]  c   Second value
/// @param[in]  d   First value
static inline Sample Fill(const float a,
                          const float b,
                          const float c,
                          const float d) {
#if (_USE_SSE)
  return _mm_set_ps(a, b, c, d);
#else
  IGNORE(a);
  IGNORE(b);
  IGNORE(c);
  return d;
#endif  // (_USE_SSE)
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
  return Fill(base + increment * 3.0f,
              base + increment * 2.0f,
              base + increment,
              base);
}

/// @brief Fill a whole Sample based on its length
///
/// The actual value is SampleSize * given value
///
/// @param[in]  base    Base value to be filled with
static inline Sample FillOnLength(const float base) {
  return Fill(base * soundtailor::SampleSize);
}

/// @brief Fill a whole Sample with the given (scalar) generator
///
/// @param[in]  generator   Generator to fill the Sample with
template <typename TypeGenerator>
static inline Sample FillWithFloatGenerator(TypeGenerator& generator) {
#if (_USE_SSE)
  return Fill(generator(),
              generator(),
              generator(),
              generator());
#else
  return Fill(generator());
#endif  // (_USE_SSE)
}

/// @brief Helper union for vectorized type to scalar array conversion
typedef union {
  Sample sample_v;  ///< Vectorized type
  float sample[soundtailor::SampleSize];  ///< Array of scalars
} ConverterFloatScalarVector;

/// @brief Extract one element from a Sample (compile-time version)
///
/// @param[in]  input   Sample to be read
// TODO(gm): faster _mm_store_ss specialization
template<unsigned i>
float GetByIndex(SampleRead input) {
#if (_USE_SSE)
  ConverterFloatScalarVector converter;
  converter.sample_v = input;
  return converter.sample[i];
#else
  return input;
#endif  // (_USE_SSE)
}

/// @brief Extract one element from a Sample (runtime version, in loops)
///
/// @param[in]  input   Sample to be read
/// @param[in]  i   Index of the element to retrieve
static inline float GetByIndex(SampleRead input, const unsigned i) {
  ASSERT(i < soundtailor::SampleSize);
  ConverterFloatScalarVector converter;
  converter.sample_v = input;
  return converter.sample[i];
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

/// @brief Add "left" to "right"
static inline Sample Add(SampleRead left, SampleRead right) {
#if (_USE_SSE)
  return _mm_add_ps(left, right);
#else
  return left + right;
#endif  // (_USE_SSE)
}

/// @brief Sum all elements of a Sample
static inline float AddHorizontal(SampleRead input) {
#if (_USE_SSE)
  const Sample first_add(Add(input, _mm_movehl_ps(input, input)));
  const Sample shuffled(_mm_shuffle_ps(first_add, first_add,
                                       _MM_SHUFFLE(0, 1, 0, 1)));
  return GetByIndex<0>(Add(shuffled, first_add));
#else
  return input;
#endif  // (_USE_SSE)
}

/// @brief Substracte "right" from "left"
static inline Sample Sub(SampleRead left, SampleRead right) {
#if (_USE_SSE)
  return _mm_sub_ps(left, right);
#else
  return left - right;
#endif  // (_USE_SSE)
}

/// @brief Add "left" to "right"
///
/// @param[in]  left   First Sample to be added
/// @param[in]  right   Second Sample to be added
static inline Sample Mul(SampleRead left, SampleRead right) {
#if (_USE_SSE)
  return _mm_mul_ps(left, right);
#else
  return left * right;
#endif  // (_USE_SSE)
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
  // TODO(gm): clarify left/right stuff since sse vectors order is not trivial
#if (_USE_SSE)
  const Sample rotated(_mm_castsi128_ps(
                       _mm_slli_si128(_mm_castps_si128(input), 4)));
  return Add(Fill(0.0f, 0.0f, 0.0f, value), rotated);
#else
  IGNORE(input);
  return value;
#endif  // (_USE_SSE)
}

/// @brief Return the sign of each element of the Sample
///
/// Sgn(0.0) return 0.0
static inline Sample Sgn(SampleRead input) {
#if (_USE_SSE)
  const Sample kZero(_mm_setzero_ps());
  const Sample kOne(Fill(1.0f));
  const Sample kMinus(Fill(-1.0f));
  const Sample kPlusMask(_mm_and_ps(_mm_cmpgt_ps(input, kZero), kOne));
  const Sample kMinusMask(_mm_and_ps(_mm_cmplt_ps(input, kZero), kMinus));
  return Add(kPlusMask, kMinusMask);
#else
  return Sub((0.0f < input), (input < 0.0f));
#endif  // (_USE_SSE)
}

/// @brief Return the sign of each element of the Sample, no zero version
///
/// Sgn(0.0) return 1.0f
static inline Sample SgnNoZero(SampleRead value) {
#if (_USE_SSE)
  const Sample kZero(_mm_setzero_ps());
  const Sample kOne(Fill(1.0f));
  const Sample kMinus(Fill(-1.0f));
  const Sample kPlusMask(_mm_and_ps(_mm_cmpge_ps(value, kZero), kOne));
  const Sample kMinusMask(_mm_and_ps(_mm_cmplt_ps(value, kZero), kMinus));
  return Add(kPlusMask, kMinusMask);
#else
  return Sub((0.0f <= value), (value < 0.0f));
#endif  // (_USE_SSE)
}

/// @brief Store the given Sample into memory
///
/// @param[in]  buffer   Memory to be filled with the input
/// @param[in]  input   Sampel to be stored
static inline void Store(float* const buffer, SampleRead input) {
#if (_USE_SSE)
  _mm_storeu_ps(buffer, input);
#else
  *buffer = input;
#endif  // (_USE_SSE)
}

#if (_USE_SSE)
/// @brief Get each right half of the two given vectors
///
/// Given left = (x0, x1, x2, x3) and right = (y0, y1, y2, y3)
/// it will return (x2, x3, y2, y3)
static inline Sample TakeEachRightHalf(SampleRead left,
                                       SampleRead right) {
  return _mm_shuffle_ps(right, left, _MM_SHUFFLE(3, 2, 3, 2));
}
#endif  // (_USE_SSE)

/// @brief Revert the given vector values order
///
/// Given value = (x0, x1, x2, x3)
/// it will return (x3, x2, x1, x0)
static inline Sample Revert(SampleRead value) {
#if (_USE_SSE)
  return _mm_shuffle_ps(value, value, _MM_SHUFFLE(0, 1, 2, 3));
#else
  return value;
#endif  // (_USE_SSE)
}

/// @brief Return each min element of both inputs
static inline Sample Min(SampleRead left, SampleRead right) {
#if (_USE_SSE)
  return _mm_min_ps(left, right);
#else
  return std::min(left, right);
#endif  // (_USE_SSE)
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
#if (_USE_SSE)
  return 15 == _mm_movemask_ps(input);
#else
  return input == 1.0f;
#endif
}

/// @brief Helper binary function: return true if all input elements are null
///
/// @param[in]  input   Input to be checked
static inline bool IsMaskNull(SampleRead input) {
#if (_USE_SSE)
  return 0 == _mm_movemask_ps(input);
#else
  return input == 0.0f;
#endif
}

/// @brief Helper binary function:
/// true if each input element is >= than the matching threshold element
static inline bool GreaterEqual(SampleRead threshold, SampleRead input) {
#if (_USE_SSE)
  const Sample test_result(_mm_cmpge_ps(threshold, input));
  return IsMaskFull(test_result);
#else
  return threshold >= input;
#endif
}

/// @brief Helper binary function:
/// true if any input element is >= than the matching threshold element
static inline bool GreaterEqualAny(SampleRead threshold, SampleRead input) {
#if (_USE_SSE)
  const Sample test_result(_mm_cmpge_ps(threshold, input));
  return !IsMaskNull(test_result);
#else
  return threshold >= input;
#endif
}


// From this point, intrinsics are no longer used,
// these are only composition of above functions


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

/// @brief Return the absolute value of each element of the Sample
static inline Sample Abs(SampleRead input) {
#if (_USE_SSE)
  return Max(Sub(Fill(0.0f), input), input);
#else
  return std::fabs(input);
#endif  // (_USE_SSE)
}

/// @brief Helper binary function:
/// true if all input elements are >= than the given threshold
#if (_USE_SSE)
static inline bool GreaterEqual(const float threshold, SampleRead input) {
  return GreaterEqual(Fill(threshold), input);
}
#endif  // (_USE_SSE)

/// @brief Helper binary function:
/// true if any input elements are >= than the given threshold
#if (_USE_SSE)
static inline bool GreaterEqualAny(const float threshold, SampleRead input) {
  return GreaterEqualAny(Fill(threshold), input);
}
#endif  // (_USE_SSE)

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

}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_MATHS_H_
