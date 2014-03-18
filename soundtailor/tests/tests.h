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

#include <cmath>

// std::generate
#include <algorithm>
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
using soundtailor::GetByIndex;
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

// Tests-specific maths (comparison operators) stuff

static inline bool IsMaskNull(SampleRead value) {
#if (_USE_SSE)
  return 0 == _mm_movemask_ps(value);
#else
  return value == 0.0f;
#endif
}

static inline bool IsMaskFull(SampleRead value) {
#if (_USE_SSE)
  return 15 == _mm_movemask_ps(value);
#else
  return value == 1.0f;
#endif
}

static inline bool GreaterThan(const float threshold, SampleRead value) {
#if (_USE_SSE)
  const Sample test_result(_mm_cmpgt_ps(Fill(threshold), value));
  return IsMaskFull(test_result);
#else
  return threshold > value;
#endif
}

static inline bool GreaterEqual(const float threshold, SampleRead value) {
#if (_USE_SSE)
  const Sample test_result(_mm_cmpge_ps(Fill(threshold), value));
  return IsMaskFull(test_result);
#else
  return threshold >= value;
#endif
}

#if (_USE_SSE)
static inline bool GreaterEqual(SampleRead threshold, SampleRead value) {
  const Sample test_result(_mm_cmpge_ps(threshold, value));
  return IsMaskFull(test_result);
}
#endif

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
static const unsigned int kIterations(16);
static const unsigned int kSignalDataPeriodsCount(32);

/// @brief Base sampling rate unless indicated otherwise
static const float kSamplingRate(96000.0f);

/// @brief Arbitrary lowest allowed fundamental
static const float kMinFundamentalNorm(10.0f / kSamplingRate);
/// @brief Arbitrary highest allowed fundamental
// TODO(gm): make this higher
static const float kMaxFundamentalNorm(2000.0f / kSamplingRate);

/// @brief Arbitrary lowest allowed key note (= A0)
static const unsigned int kMinKeyNote(21);
/// @brief Arbitrary highest allowed key note (= A6)
// TODO(gm): make this higher
static const unsigned int kMaxKeyNote(93);

/// @brief Arbitrary lowest allowed duration
static const unsigned int kMinTime(0);
/// @brief Arbitrary highest allowed duration
static const unsigned int kMaxTime(static_cast<unsigned int>(kSamplingRate));

// Smaller performance test sets in debug
#if (_BUILD_CONFIGURATION_DEBUG)
static const unsigned int kFilterDataPerfSetSize(16 * 1024);
static const unsigned int kGeneratorDataPerfSetSize(16 * 1024);
#else  // (_BUILD_CONFIGURATION_DEBUG)
static const unsigned int kFilterDataPerfSetSize(16 * 1024 * 256);
static const unsigned int kGeneratorDataPerfSetSize(16 * 1024 * 256);
#endif  // (_BUILD_CONFIGURATION_DEBUG)

/// @brief Uniform distribution of normalized frequencies
/// in ] 0.0f ; kMaxFundamentalNorm [
class NormFrequencyDistribution : public std::uniform_real_distribution<float> {
 public:
  NormFrequencyDistribution();
  ~NormFrequencyDistribution() {
    // Nothing to do here for now
  }
};
static NormFrequencyDistribution kFreqDistribution;
static std::uniform_real_distribution<float> kNormDistribution(-1.0f, 1.0f);
static std::uniform_real_distribution<float> kNormPosDistribution(0.0f, 1.0f);
static std::bernoulli_distribution kBoolDistribution;
static std::default_random_engine kRandomGenerator;

/// @brief Compute the mean value of a signal generator for the given length
///
/// @param[in]    generator      Generator to compute value from
/// @param[in]    length         Sample length
///
/// @return the generator mean for such length
template <typename TypeGenerator>
float ComputeMean(TypeGenerator& generator, const unsigned int length) {
  Sample sum(Fill(0.0f));
  unsigned int sample_idx(0);
  while (sample_idx < length) {
    const Sample sample(generator());
    sum = Add(sum, sample);
    sample_idx += soundtailor::SampleSize;
  }
  return AddHorizontal(sum) / static_cast<float>(length);
}

/// @brief Compute the mean power of a signal generator for the given length
///
/// @param[in]    generator      Generator to compute value from
/// @param[in]    length         Sample length
///
/// @return the generator mean for such length
template <typename TypeGenerator>
float ComputePower(TypeGenerator& generator, const unsigned int length) {
  Sample power(Fill(0.0f));
  unsigned int sample_idx(0);
  while (sample_idx < length) {
    const Sample sample(generator());
    const Sample squared(Mul(sample, sample));
    power = Add(power, squared);
    sample_idx += soundtailor::SampleSize;
  }
  return AddHorizontal(power) / static_cast<float>(length);
}

/// @brief Helper structure for retrieving zero crossings informations
template <typename TypeGenerator>
struct ZeroCrossing {
  explicit ZeroCrossing(TypeGenerator& generator)
      : generator_(generator),
        // TODO(gm): this may introduces an additional zero crossing,
        // it must be initialized to the first input value
        previous_sgn_(0.0f),
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

/// @brief Compute zero crossings of a signal generator for the given length
///
/// @param[in]    generator      Generator to compute value from
/// @param[in]    length         Sample length
///
/// @return zero crossings occurence for such length
template <typename TypeGenerator>
int ComputeZeroCrossing(TypeGenerator& generator, const unsigned int length) {
  ZeroCrossing<TypeGenerator> zero_crossing(generator);
  int out(0);
  unsigned int zero_crossing_idx(zero_crossing.GetNextZeroCrossing(length));
  while (zero_crossing_idx < length) {
    out += 1;
    zero_crossing_idx = zero_crossing.GetNextZeroCrossing(length);
  }
  return out;
}

/// @brief Compute the frequency of a given piano key (A4 = 440Hz)
float NoteToFrequency(const unsigned int key_number);

#endif  // SOUNDTAILOR_TESTS_TESTS_H_
