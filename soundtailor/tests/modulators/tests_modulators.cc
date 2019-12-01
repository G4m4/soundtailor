/// @file tests_modulators.cc
/// @brief SoundTailor modulators common tests
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

#include "soundtailor/tests/modulators/tests_modulators_fixture.h"

#include "soundtailor/src/modulators/adsd.h"
#include "soundtailor/src/generators/generators_common.h"

using soundtailor::modulators::Adsd;
// For testing purpose only
using soundtailor::generators::Differentiator;

/// @brief All tested types
typedef ::testing::Types<Adsd> ModulatorTypes;

TYPED_TEST_SUITE(Modulator, ModulatorTypes);
TYPED_TEST_SUITE(ModulatorData, ModulatorTypes);

/// @brief Generates an envelop, check for its range (must be >= 0)
TYPED_TEST(Modulator, Range) {
  std::cerr << "Instance size : " << sizeof(TypeParam) << std::endl;
  for (unsigned int iterations(0);
       iterations < this->kTestIterations_;
       ++iterations) {
    IGNORE(iterations);

    TypeParam generator;
    generator.SetParameters(this->kAttack_,
                            this->kDecay_,
                            this->kDecay_,
                            this->kSustainLevel_);

    const float kEpsilon(1e-3f);

    generator.TriggerOn();
    unsigned int i(0);
    // Attack / Decay / Sustain
    while (i <= this->kAttack_ + this->kDecay_ + this->kSustain_) {
      const Sample sample(generator());
      const bool lower_bound(soundtailor::VectorMath::LessEqual(0.0f - kEpsilon, sample));
      const bool upper_bound(soundtailor::VectorMath::GreaterEqual(1.0f + kEpsilon, sample));
      EXPECT_TRUE(lower_bound);
      EXPECT_TRUE(upper_bound);
      i += soundtailor::SampleSize;
    }
    // Release + a little bit after that
    generator.TriggerOff();
    while (i <= this->kAttack_
                + this->kDecay_
                + this->kSustain_
                + this->kDecay_
                + this->kTail_) {
      const Sample sample(generator());
      const bool lower_bound(soundtailor::VectorMath::LessEqual(0.0f - kEpsilon, sample));
      const bool upper_bound(soundtailor::VectorMath::GreaterEqual(1.0f + kEpsilon, sample));
      EXPECT_TRUE(lower_bound);
      EXPECT_TRUE(upper_bound);
      i += soundtailor::SampleSize;
    }
  }  // iterations?
}

/// @brief Generates an envelop, check for its proper timings:
/// - when in attack samples should be in a continuous upward slope
/// - when in decay samples should be in a continuous downward slope
/// - when in sustain samples should all be equal
/// - when in release samples should be in a continuous downward slope
TYPED_TEST(Modulator, Timings) {
  for (unsigned int iterations(0);
       iterations < this->kTestIterations_;
       ++iterations) {
    IGNORE(iterations);

    TypeParam generator;
    generator.SetParameters(this->kAttack_,
                            this->kDecay_,
                            this->kDecay_,
                            this->kSustainLevel_);

    generator.TriggerOn();
    std::vector<unsigned int> zero_crossing_indexes;

    // TODO(gm): get rid of that
    typename Modulator<TypeParam>::AdsdFunctor modulators_functor(&generator);
    ZeroCrossing<  typename Modulator<TypeParam>::AdsdFunctor> zero_crossing(modulators_functor, 1.0f);
    unsigned int kTriggerOnLength(this->kAttack_ + this->kDecay_ + this->kSustain_);
    unsigned int kTotalLength(kTriggerOnLength + this->kDecay_ + this->kTail_);
    // A tiny delay occurs due to differentiation and trigger unevenness
    unsigned int kEpsilon(2);
    unsigned int zero_crossing_idx(
      zero_crossing.GetNextZeroCrossing(kTriggerOnLength));
    while (zero_crossing_idx < kTriggerOnLength) {
      zero_crossing_indexes.push_back(zero_crossing_idx);
      zero_crossing_idx = zero_crossing.GetNextZeroCrossing(kTriggerOnLength);
    }
    generator.TriggerOff();
    while (zero_crossing_idx < kTotalLength) {
      zero_crossing_idx = zero_crossing.GetNextZeroCrossing(kTotalLength);
      zero_crossing_indexes.push_back(zero_crossing_idx);
    }
    EXPECT_NEAR(this->kAttack_, zero_crossing_indexes[0], kEpsilon);
    EXPECT_NEAR(this->kAttack_ + this->kDecay_,
                zero_crossing_indexes[1],
                kEpsilon);
    EXPECT_NEAR(kTriggerOnLength, zero_crossing_indexes[2], kEpsilon);
    EXPECT_NEAR(kTriggerOnLength + this->kDecay_,
                zero_crossing_indexes[3],
                kEpsilon);
  }  // iterations?
}

/// @brief Check timings for long slopes
///
/// With the parameters used here, the slope is still supposed
/// to be > FLT_EPSILON
TYPED_TEST(Modulator, LongTimings) {
  const unsigned int kLongTime(this->kMaxTime_);
  const unsigned int kAttack(kLongTime);
  const unsigned int kDecay(kLongTime);
  const unsigned int kSustain(100);

  TypeParam generator;
  generator.SetParameters(kAttack, kDecay, kDecay, this->kSustainLevel_);

  generator.TriggerOn();
  std::vector<unsigned int> zero_crossing_indexes;

  // TODO(gm): get rid of that
  typename Modulator<TypeParam>::AdsdFunctor modulators_functor(&generator);
  ZeroCrossing<typename Modulator<TypeParam>::AdsdFunctor> zero_crossing(modulators_functor);
  unsigned int kTriggerOnLength(kAttack + kDecay + kSustain);
  unsigned int kTotalLength(kTriggerOnLength + kDecay + this->kTail_);
  // A tiny delay occurs due to differentiation and trigger unevenness
  unsigned int kEpsilon(2);
  unsigned int zero_crossing_idx(
    zero_crossing.GetNextZeroCrossing(kTriggerOnLength));
  while (zero_crossing_idx < kTriggerOnLength) {
    zero_crossing_indexes.push_back(zero_crossing_idx);
    zero_crossing_idx = zero_crossing.GetNextZeroCrossing(kTriggerOnLength);
  }
  generator.TriggerOff();
  while (zero_crossing_idx < kTotalLength) {
    zero_crossing_idx = zero_crossing.GetNextZeroCrossing(kTotalLength);
    zero_crossing_indexes.push_back(zero_crossing_idx);
  }
  EXPECT_NEAR(kAttack, zero_crossing_indexes[0], kEpsilon);
  EXPECT_NEAR(kAttack + kDecay, zero_crossing_indexes[1], kEpsilon);
  EXPECT_NEAR(kTriggerOnLength, zero_crossing_indexes[2], kEpsilon);
  EXPECT_NEAR(kTriggerOnLength + kDecay, zero_crossing_indexes[3], kEpsilon);
}

/// @brief Generates an envelop with one or both timing parameters null
TYPED_TEST(Modulator, NullParameters) {
  for (unsigned int iterations(0);
       iterations < this->kTestIterations_;
       ++iterations) {
    IGNORE(iterations);

    // Random parameters
    // Each parameter has half a chance to be null
    const unsigned int kAttack(kBoolDistribution(this->kRandomGenerator_)
                               ? this->kAttack_ : 0);
    const unsigned int kDecay(kBoolDistribution(this->kRandomGenerator_)
                              ? this->kDecay_ : 0);

    TypeParam generator;
    generator.SetParameters(kAttack, kDecay, kDecay, this->kSustainLevel_);

    const float kEpsilon(1e-3f);

    generator.TriggerOn();
    unsigned int i(1);
    Differentiator differentiator;
    while (i <= kAttack) {
      const Sample diff(differentiator(generator()));
      const bool is_increasing(soundtailor::VectorMath::LessEqual(0.0f, diff));
      EXPECT_TRUE(is_increasing);
      i += soundtailor::SampleSize;
    }
    // If the attack is null, then we have to ignore its "click".
    if (0 == kAttack) {
      differentiator(generator());
    } else {
      // It might happen that the decreasing stage is 1 sample off
      const Sample diff_initial(differentiator(generator()));
      EXPECT_GE(0.0f, VectorMath::GetByIndex<1>(diff_initial));
      EXPECT_GE(0.0f, VectorMath::GetByIndex<2>(diff_initial));
      EXPECT_GE(0.0f, VectorMath::GetByIndex<3>(diff_initial));
      i += soundtailor::SampleSize;
    }
    while (i <= kAttack + kDecay) {
      const Sample diff(differentiator(generator()));
      // Small epsilon in case of a tiny jump from the current to the sustain value
      const bool is_decreasing(soundtailor::VectorMath::GreaterEqual(1e-7f, diff));
      EXPECT_TRUE(is_decreasing);
      i += soundtailor::SampleSize;
    }
    while (i < kAttack + kDecay + this->kSustain_ + soundtailor::SampleSize - 1) {
      // A (really tiny) epsilon is required here for imprecisions
      const float kNearEpsilon(1e-6f);
      const bool is_near(soundtailor::VectorMath::IsNear(
        soundtailor::VectorMath::Fill(this->kSustainLevel_), generator(), kNearEpsilon));
      EXPECT_TRUE(is_near);
      i += soundtailor::SampleSize;
    }
    generator.TriggerOff();
    while (i < kAttack + kDecay + this->kSustain_ + kDecay + this->kTail_) {
      const Sample sample(generator());
      const bool lower_bound(soundtailor::VectorMath::LessEqual(0.0f - kEpsilon, sample));
      const bool upper_bound(soundtailor::VectorMath::GreaterEqual(1.0f + kEpsilon, sample));
      EXPECT_TRUE(lower_bound);
      EXPECT_TRUE(upper_bound);
      i += soundtailor::SampleSize;
    }
  }  // iterations?
}

/// @brief Generates a "click envelop" - with both timing parameters null
TYPED_TEST(Modulator, Click) {
  for (unsigned int iterations(0);
       iterations < this->kTestIterations_;
       ++iterations) {
    IGNORE(iterations);

    const unsigned int kAttack(0);
    const unsigned int kDecay(0);

    TypeParam generator;
    generator.SetParameters(kAttack, kDecay, kDecay, this->kSustainLevel_);

    generator.TriggerOn();
    unsigned int i(1);
    // The first sample is always null!
    soundtailor::IGNORE(generator());
    while (i <= this->kSustain_) {
      const Sample sample(generator());
      // A (really tiny) epsilon is required here for imprecisions
      const float kEpsilon(1e-6f);
      const bool is_near(soundtailor::VectorMath::IsNear(
        soundtailor::VectorMath::Fill(this->kSustainLevel_), sample, kEpsilon));
      EXPECT_TRUE(is_near);
      i += soundtailor::SampleSize;
    }
    generator.TriggerOff();
    // This sample left is due to the release
    soundtailor::IGNORE(generator());
    while (i <= this->kSustain_ + this->kTail_) {
      const float kEpsilon(1e-6f);
      const bool is_null(soundtailor::VectorMath::IsNear(
        soundtailor::VectorMath::Fill(0.0f), generator(), kEpsilon));
      EXPECT_TRUE(is_null);
      i += soundtailor::SampleSize;
    }
  }  // iterations?
}

/// @brief Generates an envelop, check for its "regularity"
/// the more regular it is, the less spiky, the less liable to audio artefacts
/// on the modulated signal
/// Checking regularity is done as follows: since timings are parameterized,
/// it is easy to deduce the maximum theoretical change
/// between two consecutive samples. Nowhere it should be greater!
TYPED_TEST(Modulator, OutRegularity) {
  for (unsigned int iterations(0);
       iterations < this->kTestIterations_;
       ++iterations) {
    IGNORE(iterations);

    TypeParam generator;
    generator.SetParameters(this->kAttack_,
                            this->kDecay_,
                            this->kDecay_,
                            this->kSustainLevel_);

    generator.TriggerOn();
    unsigned int i(1);
    // A very small epsilon is added for computation/casts imprecisions
    // TODO(gm): this might not be required if all floating point operations
    // were properly understood and managed.
    const float kMaxDelta(static_cast<float>(1.0 / std::min(this->kAttack_, this->kDecay_) + 1e-7));
    // Envelops should all begin at zero!
    Differentiator differentiator;
    // Checking the whole envelop since clicks may occur anywhere
    while (i < this->kAttack_ + this->kDecay_ + this->kSustain_) {
      const Sample diff(differentiator(generator()));
      const bool is_small(soundtailor::VectorMath::GreaterEqual(kMaxDelta, diff));
      EXPECT_TRUE(is_small);
      i += soundtailor::SampleSize;
    }
    generator.TriggerOff();
    while (i < this->kAttack_ + this->kDecay_ + this->kSustain_ + this->kDecay_ + this->kTail_) {
      const Sample diff(differentiator(generator()));
      const bool is_small(soundtailor::VectorMath::GreaterEqual(kMaxDelta, diff));
      EXPECT_TRUE(is_small);
      i += soundtailor::SampleSize;
    }
  }  // iterations?
}

/// @brief Check that both per-sample and per-block generation methods
/// yield an identical result
TYPED_TEST(ModulatorData, Process) {
  TypeParam generator_perblock;
  TypeParam generator_persample;
  // Random parameters
  // Each parameter has half a chance to be null
  const unsigned int kAttack(kBoolDistribution(this->kRandomGenerator_)
                             ? this->kAttack_ : 0);
  const unsigned int kDecay(kBoolDistribution(this->kRandomGenerator_)
                            ? this->kDecay_ : 0);
  generator_perblock.SetParameters(kAttack,
                                   kDecay,
                                   kDecay,
                                   this->kSustainLevel_);
  generator_persample.SetParameters(kAttack,
                                    kDecay,
                                    kDecay,
                                    this->kSustainLevel_);
  generator_perblock.TriggerOn();
  generator_persample.TriggerOn();

  soundtailor::ProcessBlock(&this->output_data_[0],
                            this->output_data_.size(),
                            generator_perblock);
  for (unsigned int i(0); i < this->kModulatorDataPerfSetSize_; i += soundtailor::SampleSize) {
    const Sample kReference(VectorMath::Fill(&this->output_data_[i]));
    const Sample kGenerated((generator_persample()));
    EXPECT_TRUE(VectorMath::Equal(kReference, kGenerated));
  }
}

/// @brief Generates an envelop (performance test)
// Here the tested length cannot be longer in release configuration,
// because it would then only test the performance on the envelop tail!
// That's why we are using a "kModulatorPerfIterations", adding iterations
// when in Release.
TYPED_TEST(Modulator, Perf) {
  for (unsigned int iterations(0);
       iterations < this->kPerfIterations_;
       ++iterations) {
    IGNORE(iterations);

    TypeParam generator;
    // Random parameters
    // Each parameter has half a chance to be null
    const unsigned int kAttack(kBoolDistribution(this->kRandomGenerator_)
                               ? this->kAttack_ : 0);
    const unsigned int kDecay(kBoolDistribution(this->kRandomGenerator_)
                              ? this->kDecay_ : 0);
    generator.SetParameters(kAttack,
                            kDecay,
                            kDecay,
                            this->kSustainLevel_);
    generator.TriggerOn();

    unsigned int sample_idx(0);
    while (sample_idx < this->kAttack_ + this->kDecay_ + this->kSustain_) {
      const Sample kCurrent(generator());
      sample_idx += soundtailor::SampleSize;
      // No actual test!
      const bool lower_bound(soundtailor::VectorMath::LessEqual(-1.0f, kCurrent));
      EXPECT_TRUE(lower_bound);
    }
    generator.TriggerOff();
    while (sample_idx < this->kModulatorDataPerfSetSize_) {
      const Sample kCurrent(generator());
      sample_idx += soundtailor::SampleSize;
      // No actual test!
      const bool lower_bound(soundtailor::VectorMath::LessEqual(-1.0f, kCurrent));
      EXPECT_TRUE(lower_bound);
    }
  }  // iterations?
}

/// @brief Generates an envelop (block performance tests)
TYPED_TEST(ModulatorData, BlockPerf) {
  for (unsigned int iterations(0); iterations < this->kPerfIterations_; ++iterations) {
    IGNORE(iterations);

    TypeParam generator;
    // Random parameters
    // Each parameter has half a chance to be null
    const unsigned int kAttack(kBoolDistribution(this->kRandomGenerator_)
                               ? this->kAttack_ : 0);
    const unsigned int kDecay(kBoolDistribution(this->kRandomGenerator_)
                              ? this->kDecay_ : 0);
    generator.SetParameters(kAttack,
                            kDecay,
                            kDecay,
                            this->kSustainLevel_);
    generator.TriggerOn();

    soundtailor::ProcessBlock(&this->output_data_[0],
                              this->output_data_.size(),
                              generator);
    unsigned int sample_idx(0);
    while (sample_idx < this->kModulatorDataPerfSetSize_) {
      const Sample kCurrent(VectorMath::Fill(&this->output_data_[sample_idx]));
      sample_idx += soundtailor::SampleSize;
      // No actual test!
      EXPECT_TRUE(VectorMath::LessEqual(-2.0f, kCurrent));
    }
  }
}
