/// @file tests.h
/// @brief Tests common include file
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

#ifndef SOUNDTAILOR_TESTS_TESTS_H_
#define SOUNDTAILOR_TESTS_TESTS_H_

// std::generate
#include <algorithm>
// std::bind
#include <functional>
#include <random>

#include "gtest/gtest.h"

#include "soundtailor/src/common.h"
#include "soundtailor/src/configuration.h"
#include "soundtailor/src/maths.h"

using soundtailor::IGNORE;

// Using declarations for soundtailor maths stuff
using soundtailor::Sample;
using soundtailor::SampleRead;
using soundtailor::Fill;
using soundtailor::FillWithFloatGenerator;
using soundtailor::GetByIndex;
using soundtailor::GetLast;
using soundtailor::GetFirst;
using soundtailor::Add;
using soundtailor::AddHorizontal;
using soundtailor::Sub;
using soundtailor::Mul;
using soundtailor::MulConst;
using soundtailor::Abs;
using soundtailor::Sgn;
using soundtailor::SgnNoZero;
using soundtailor::Store;
using soundtailor::Round;
using soundtailor::IsMaskFull;
using soundtailor::IsMaskNull;
using soundtailor::GreaterEqual;
using soundtailor::IsNear;
using soundtailor::RotateOnRight;

// Tests-specific maths (comparison operators) stuff

static inline bool GreaterThan(const float threshold, SampleRead value) {
#if (_USE_SSE)
  const Sample test_result(_mm_cmpgt_ps(Fill(threshold), value));
  return IsMaskFull(test_result);
#else
  return threshold > value;
#endif
}

static inline bool LessThan(const float threshold, SampleRead value) {
#if (_USE_SSE)
  const Sample test_result(_mm_cmplt_ps(Fill(threshold), value));
  return IsMaskFull(test_result);
#else
  return threshold < value;
#endif
}

static inline bool LessEqual(const float threshold, SampleRead value) {
#if (_USE_SSE)
  const Sample test_result(_mm_cmple_ps(Fill(threshold), value));
  return IsMaskFull(test_result);
#else
  return threshold <= value;
#endif
}

#if (_USE_SSE)
static inline bool LessEqual(SampleRead threshold, SampleRead value) {
  const Sample test_result(_mm_cmple_ps(threshold, value));
  return IsMaskFull(test_result);
}
#endif

static inline bool Equal(const float threshold, SampleRead value) {
#if (_USE_SSE)
  const Sample test_result(_mm_cmpeq_ps(Fill(threshold), value));
  return IsMaskFull(test_result);
#else
  return threshold == value;
#endif
}

#if (_USE_SSE)
static inline bool Equal(SampleRead threshold, SampleRead value) {
  const Sample test_result(_mm_cmpeq_ps(threshold, value));
  return IsMaskFull(test_result);
}
#endif

static const unsigned int kDataTestSetSize(32768);

/// @brief Base sampling rate unless indicated otherwise
static const float kSamplingRate(96000.0f);

static std::uniform_real_distribution<float> kNormDistribution(-1.0f, 1.0f);
static std::uniform_real_distribution<float> kNormPosDistribution(0.0f, 1.0f);
static std::bernoulli_distribution kBoolDistribution;
static std::default_random_engine kRandomGenerator;

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
    const Sample sign_v(SgnNoZero(input));
    for (unsigned int index(0); index < soundtailor::SampleSize; index += 1) {
      const float current_sgn(GetByIndex(sign_v, index));
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

#endif  // SOUNDTAILOR_TESTS_TESTS_H_
