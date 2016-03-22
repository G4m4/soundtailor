/// @file analysis.h
/// @brief Signal analysis utility
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

#ifndef SOUNDTAILOR_TESTS_ANALYSIS_H_
#define SOUNDTAILOR_TESTS_ANALYSIS_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/maths.h"

namespace soundtailor {

/// @brief Helper structure for retrieving zero crossings informations
template <typename TypeGenerator>
struct ZeroCrossing {
  explicit ZeroCrossing(TypeGenerator& generator,
                        const float previous_sgn = 1.0f)
    : generator_(generator),
    previous_sgn_(previous_sgn),
    cursor_(0) {
    // Nothing to do here
  }

  /// @brief Get next zero crossing absolute index
  unsigned int GetNextZeroCrossing(unsigned int max_length) {
    while (cursor_ < max_length) {
      int index_zc(GetZeroCrossingRelative(generator_()));
      if (index_zc >= 0) {
        const unsigned int out(index_zc + cursor_);
        cursor_ += soundtailor::SampleSize;
        return out;
      }
      cursor_ += soundtailor::SampleSize;
    }
    return max_length;
  }

  unsigned int Cursor(void) {
    return cursor_;
  }

private:
  /// @brief Actual zero crossing detection method
  ///
  /// Beware, it cannot detect zero crossings closer than 4 samples!
  /// TODO(gm): Fix it
  ///
  /// @return the (relative) index of the next zero crossing, or -1
  int GetZeroCrossingRelative(Sample input) {
    const Sample sign_v(VectorMath::SgnNoZero(input));
    for (unsigned int index(0); index < soundtailor::SampleSize; index += 1) {
      const float current_sgn(VectorMath::GetByIndex(sign_v, index));
      if (previous_sgn_ != current_sgn) {
        previous_sgn_ = current_sgn;
        return index;
      }
      previous_sgn_ = current_sgn;
    }
    return -1;
  }

  TypeGenerator generator_;
  float previous_sgn_;
  unsigned int cursor_;
};

/// @brief Compute the mean value of a signal generator for the given length
///
/// @param[in]    generator      Generator to compute value from
/// @param[in]    length         Sample length
///
/// @return the generator mean for such length
template <typename TypeGenerator>
float ComputeMean(TypeGenerator& generator, const unsigned int length) {
  Sample sum(VectorMath::Fill(0.0f));
  unsigned int sample_idx(0);
  while (sample_idx < length) {
    const Sample sample(generator());
    sum = VectorMath::Add(sum, sample);
    sample_idx += soundtailor::SampleSize;
  }
  return VectorMath::AddHorizontal(sum) / static_cast<float>(length);
}

/// @brief Compute the mean power of a signal generator for the given length
///
/// @param[in]    generator      Generator to compute value from
/// @param[in]    length         Sample length
///
/// @return the generator mean for such length
template <typename TypeGenerator>
float ComputePower(TypeGenerator& generator, const unsigned int length) {
  Sample power(VectorMath::Fill(0.0f));
  unsigned int sample_idx(0);
  while (sample_idx < length) {
    const Sample sample(generator());
    const Sample squared(VectorMath::Mul(sample, sample));
    power = VectorMath::Add(power, squared);
    sample_idx += soundtailor::SampleSize;
  }
  return VectorMath::AddHorizontal(power) / static_cast<float>(length);
}

/// @brief Compute zero crossings of a signal generator for the given length
///
/// @param[in]  generator   Generator to compute value from
/// @param[in]  length    Sample length
/// @param[in]  initial_sgn   Initial generator sign, useful for generators
///                           beginning at 0 and decreasing (Triangle DPW...)
///
/// @return zero crossings occurence for such length
template <typename TypeGenerator>
int ComputeZeroCrossing(TypeGenerator& generator,
                        const unsigned int length,
                        const float initial_sgn = 1.0f) {
  ZeroCrossing<TypeGenerator> zero_crossing(generator, initial_sgn);
  int out(0);
  unsigned int zero_crossing_idx(zero_crossing.GetNextZeroCrossing(length));
  while (zero_crossing_idx < length) {
    out += 1;
    zero_crossing_idx = zero_crossing.GetNextZeroCrossing(length);
  }
  return out;
}

/// @brief Helper structure for checking a signal continuity
struct IsContinuous {
  /// @brief Default constructor
  ///
  /// @param[in]  threshold   Max difference between two consecutive samples
  /// @param[in]  previous   First sample initialization
  IsContinuous(const float threshold, const float previous)
      : threshold_(threshold),
        previous_(previous) {
    SOUNDTAILOR_ASSERT(threshold >= 0.0f);
  }

  /// @brief Check next sample continuity
  ///
  /// @param[in]  input   Sample to be tested
  bool operator()(SampleRead input) {
    const float before_diff(VectorMath::GetLast(input));
    const Sample prev(VectorMath::RotateOnRight(input, previous_));
    const Sample after_diff(VectorMath::Sub(input, prev));
    previous_ = before_diff;
    if (VectorMath::LessThan(threshold_, VectorMath::Abs(after_diff))) {
      return false;
    }
    return true;
  }

  float threshold_;
  float previous_;
};

}  // namespace soundtailor

#endif  // SOUNDTAILOR_TESTS_ANALYSIS_H_
