/// @file tests_generators.cc
/// @brief SoundTailor generators common tests
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

#include "soundtailor/tests/generators/tests_generators_fixture.h"

#include "soundtailor/src/generators/generators_common.h"
#include "soundtailor/src/generators/sawtooth_dpw.h"
#include "soundtailor/src/generators/triangle_dpw.h"

using soundtailor::generators::PhaseAccumulator;
using soundtailor::generators::SawtoothDPW;
using soundtailor::generators::TriangleDPW;

/// @brief All tested types
typedef ::testing::Types<PhaseAccumulator,
                         SawtoothDPW,
                         TriangleDPW> GeneratorTypes;

TYPED_TEST_CASE(Generator, GeneratorTypes);
TYPED_TEST_CASE(GeneratorData, GeneratorTypes);

/// @brief Generates a signal, check for null mean (no DC offset)
TYPED_TEST(Generator, Mean) {
  for (unsigned int iterations(0); iterations < this->kTestIterations; ++iterations) {
    IGNORE(iterations);
    // Random normalized frequency
    const float kFrequency(this->kFreqDistribution(this->kRandomGenerator));

    // We are generating complete periods to prevent false positive
    const unsigned int kDataLength(ComputeDataLength(kFrequency,
                                                     this->kSignalDataPeriodsCount));

    // Generating data
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    const float kExpected(0.0f);
    // Epsilon is quite big here, this generator being very crude
    const float kEpsilon(1e-1f);
    const float kActual(ComputeMean(generator, kDataLength));

    EXPECT_NEAR(kExpected, kActual, kEpsilon);
  }  // iterations?
}

/// @brief Generates a signal, check for signal power
TYPED_TEST(Generator, Power) {
  for (unsigned int iterations(0); iterations < this->kTestIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(this->kFreqDistribution(this->kRandomGenerator));

    // We are generating complete periods to prevent false positive
    const unsigned int kDataLength(ComputeDataLength(kFrequency,
                                                     this->kSignalDataPeriodsCount));

    // Generating data
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    const float kExpected(1.0f / 3.0f);
    // Very low epsilon with this algorithm!
    const float kEpsilon(6e-3f);
    const float kActual(ComputePower(generator, kDataLength));

    EXPECT_NEAR(kExpected, kActual, kEpsilon);
  }  // iterations?
}

/// @brief Generates a signal,
/// check for normalized range (within [-1.0f ; 1.0f])
TYPED_TEST(Generator, Range) {
  for (unsigned int iterations(0); iterations < this->kTestIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(this->kFreqDistribution(this->kRandomGenerator));
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    for (unsigned int i(0);
         i < this->kDataTestSetSize;
         i += soundtailor::SampleSize) {
      const Sample sample(Fill(this->kFreqDistribution(this->kRandomGenerator)));
      EXPECT_TRUE(GreaterEqual(1.0f, sample));
      EXPECT_TRUE(LessEqual(-1.0f, sample));
    }
  }
}

/// @brief Generates a signal and check for expected zero crossing
/// according parameterized frequency (1 expected zero crossings per period)
TYPED_TEST(Generator, ZeroCrossings) {
  for (unsigned int iterations(0); iterations < this->kTestIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(this->kFreqDistribution(this->kRandomGenerator));
    // Adding half a period makes the test more robusts to few samples shifts
    const unsigned int kDataLength(ComputeDataLength(
                                     kFrequency,
                                     this->kSignalDataPeriodsCount + 0.5f));
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    const float kEpsilon(1.0f);
    const float kActual(static_cast<float>(ComputeZeroCrossing(generator,
                                                               kDataLength)));

    EXPECT_NEAR(this->kSignalDataPeriodsCount, kActual, kEpsilon);
  }
}

/// @brief Generates a signal at each frequency corresponding
/// to key notes in the available range, check for expected zero crossing
TYPED_TEST(Generator, Notes) {
  for (unsigned int key_note(this->kMinKeyNote);
       key_note < this->kMaxKeyNote;
       ++key_note) {
    const float kFrequency(NoteToFrequency(key_note));
    const unsigned int kDataLength(ComputeDataLength(kFrequency / this->kSamplingRate,
                                                     this->kSignalDataPeriodsCount));
    SawtoothDPW generator;
    generator.SetFrequency(kFrequency / this->kSamplingRate);

    // Due to rounding one or even two zero crossings may be lost/added
    const int kEpsilon(2);
    const int kActual(ComputeZeroCrossing(generator, kDataLength));

    EXPECT_NEAR(this->kSignalDataPeriodsCount, kActual, kEpsilon);
  }
}

/// @brief Generate half a signal with a generator, use another generator
/// for the other half by forcing the second generator phase.
/// No difference should be perceptible at the transition
TYPED_TEST(Generator, PhaseControl) {
  for (unsigned int iterations(0); iterations < this->kTestIterations; ++iterations) {
    IGNORE(iterations);
    const float kFrequency(this->kFreqDistribution(this->kRandomGenerator));

    // The history must be a non-integer number of periods:
    // this prevent having the transition falls on the period beginning/ending
    const float kSignalDataPeriod(1.3f);
    const unsigned int kHistoryLength(ComputeDataLength(kFrequency,
                                                        kSignalDataPeriod));

    // Generating data
    PhaseAccumulator generator_left;
    PhaseAccumulator generator_right;
    generator_left.SetFrequency(kFrequency);
    generator_right.SetFrequency(kFrequency);

    // A small epsilon is added for differentiation imprecisions
    const float kMaxDelta(4.0f * kFrequency + 5e-5f);

    // Creating an history
    Sample sample(Fill(0.0f));
    for (unsigned int i(0);
         i < kHistoryLength;
         i += soundtailor::SampleSize) {
      sample = generator_left();
    }
    // The transition may fall anywhere!
    unsigned int kTransitionIndex(kHistoryLength % soundtailor::SampleSize);
    if (kTransitionIndex != 0) {
      kTransitionIndex -= 1;
    }
    // Forcing right generator phase
    const float current_phase(GetByIndex(sample, kTransitionIndex));
    generator_right.SetPhase(current_phase);
    // Set phase AFTER frequency allows to check if the transition is OK,
    // whatever the parameterization order
    generator_right.SetFrequency(kFrequency);
    // This is required in order to clear the generator history
    generator_right.ProcessParameters();
    IsContinuous is_continuous(kMaxDelta, current_phase);
    // Check the next 4 Samples for continuity:
    // only the transition is interesting here
    for (unsigned int i(kHistoryLength);
         i < kHistoryLength + 4;
         i += soundtailor::SampleSize) {
      EXPECT_TRUE(is_continuous(generator_right()));
    }
  }  // iterations?
}

/// @brief Check that the first generated sample is always a zero
TYPED_TEST(Generator, BeginsAtZero) {
  for (unsigned int iterations(0); iterations < this->kTestIterations; ++iterations) {
    IGNORE(iterations);
    const float kFrequency(this->kFreqDistribution(this->kRandomGenerator));

    // Generating data
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);
    const float first_sample(GetFirst(generator()));
    EXPECT_EQ(0.0f, first_sample);
  }  // iterations?
}

/// @brief Check that both per-sample and per-block generation methods
/// yield an identical result
TYPED_TEST(GeneratorData, Process) {
  // Random normalized frequency
  const float kFrequency(this->kFreqDistribution(this->kRandomGenerator));

  PhaseAccumulator generator_perblock;
  PhaseAccumulator generator_persample;
  generator_perblock.SetFrequency(kFrequency);
  generator_persample.SetFrequency(kFrequency);

  generator_perblock.ProcessBlock(&this->output_data_[0], this->output_data_.size());
  for (unsigned int i(0); i < this->kDataTestSetSize; i += soundtailor::SampleSize) {
    const Sample kReference(Fill(&this->output_data_[i]));
    const Sample kGenerated((generator_persample()));
    EXPECT_TRUE(Equal(kReference, kGenerated));
  }
}

/// @brief Generates a signal (performance tests)
TYPED_TEST(Generator, Perf) {
  for (unsigned int iterations(0); iterations < this->kPerfIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(this->kFreqDistribution(this->kRandomGenerator));
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    unsigned int sample_idx(0);
    while (sample_idx < this->kDataTestSetSize) {
      const Sample kCurrent(Fill(this->kFreqDistribution(this->kRandomGenerator)));
      sample_idx += soundtailor::SampleSize;
      // No actual test!
      EXPECT_TRUE(LessEqual(-2.0f, kCurrent));
    }
  }
}

/// @brief Generates a signal (block performance tests)
TYPED_TEST(GeneratorData, BlockPerf) {
  for (unsigned int iterations(0); iterations < this->kPerfIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(this->kFreqDistribution(this->kRandomGenerator));
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    generator.ProcessBlock(&this->output_data_[0], this->output_data_.size());
    unsigned int sample_idx(0);
    while (sample_idx < this->kDataTestSetSize) {
      const Sample kCurrent(Fill(&this->output_data_[sample_idx]));
      sample_idx += soundtailor::SampleSize;
      // No actual test!
      EXPECT_TRUE(LessEqual(-2.0f, kCurrent));
    }
  }
}
