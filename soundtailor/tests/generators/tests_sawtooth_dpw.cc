/// @file tests_sawtooth_dpw.cc
/// @brief Sawtooth DPW generator specific tests
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

#include "soundtailor/tests/tests.h"

#include "soundtailor/src/generators/sawtooth_dpw.h"

// Using declarations for tested generator
using soundtailor::generators::SawtoothDPW;

/// @brief Generates a signal, check for null mean (no DC offset)
TEST(Generators, SawtoothDPWMean) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // We are generating complete periods to prevent false positive
    const unsigned int kDataLength(static_cast<unsigned int>(
                                     std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriodsCount)));

    // Generating data
    SawtoothDPW generator;
    generator.SetFrequency(kFrequency);

    const float kExpected(0.0f);
    const float kEpsilon(1e-2f);
    const float kActual(ComputeMean(generator, kDataLength));

    std::cout << "Frequency: " << kFrequency
              << "    Mean:" << kActual << std::endl;

    EXPECT_NEAR(kExpected, kActual, kEpsilon);
  }  // iterations?
}

/// @brief Generates a signal, check for its mean power
TEST(Generators, SawtoothDPWPower) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // We are generating complete periods to prevent false positive
    const unsigned int kDataLength(static_cast<unsigned int>(
                                     std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriodsCount)));

    // Generating data
    SawtoothDPW generator;
    generator.SetFrequency(kFrequency);

    const float kExpected(1.0f / 3.0f);
    // Sawtooth is less powerful than expected using this algorithm
    const float kEpsilon(1e-1f);
    const float kActual(ComputePower(generator, kDataLength));

    std::cout << "Frequency: " << kFrequency
              << "    Power:" << kActual << std::endl;
    EXPECT_NEAR(kExpected, kActual, kEpsilon);
  }  // iterations?
}

/// @brief Generates a signal,
/// check for normalized range (within [-1.0f ; 1.0f])
TEST(Generators, SawtoothDPWRange) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(kFreqDistribution(kRandomGenerator));

    SawtoothDPW generator;
    generator.SetFrequency(kFrequency);

    for (unsigned int i(0);
         i < kDataTestSetSize;
         i += soundtailor::SampleSize) {
      const Sample sample(generator());
      EXPECT_TRUE(GreaterEqual(1.0f, sample));
      EXPECT_TRUE(LessEqual(-1.0f, sample));
    }
  }
}

/// @brief Generates a signal and check for expected zero crossing
/// according parameterized frequency (1 expected zero crossings per period)
TEST(Generators, SawtoothDPWZeroCrossings) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

  const float kFrequency(kFreqDistribution(kRandomGenerator));

    const unsigned int kDataLength(static_cast<unsigned int>(
                                     std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriodsCount)));
    SawtoothDPW generator;
    generator.SetFrequency(kFrequency);

    // Due to rounding one or even two zero crossings may be lost/added
    const int kEpsilon(2);
    const int kActual(ComputeZeroCrossing(generator, kDataLength));

    EXPECT_NEAR(kSignalDataPeriodsCount, kActual, kEpsilon);
  }
}

/// @brief Generates a signal at each frequency corresponding
/// to key notes in the available range, check for expected zero crossing
TEST(Generators, SawtoothDPWNotes) {
  for (unsigned int key_note(kMinKeyNote);
       key_note < kMaxKeyNote;
       ++key_note) {
    const float kFrequency(NoteToFrequency(key_note));
    const unsigned int kDataLength(
       static_cast<unsigned int>(std::floor((0.5f / kFrequency)
                                            * kSignalDataPeriodsCount
                                            * kSamplingRate)));
    SawtoothDPW generator;
    generator.SetFrequency(kFrequency / kSamplingRate);

    // Due to rounding one or even two zero crossings may be lost/added
    const int kEpsilon(2);
    const int kActual(ComputeZeroCrossing(generator, kDataLength));

    EXPECT_NEAR(kSignalDataPeriodsCount, kActual, kEpsilon);
  }
}

/// @brief Generate half a signal with a generator, use another generator
/// for the other half by forcing the second generator phase.
/// No difference should be perceptible at the transition
TEST(Generators, SawtoothDPWPhaseControl) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // The history must be a non-integer number of periods:
    // this prevent having the transition falls on the period beginning/ending
    const float kSignalDataPeriod(1.3f);
    const unsigned int kHistoryLength(static_cast<unsigned int>(
                                        std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriod)));

    // Generating data
    SawtoothDPW generator_left;
    SawtoothDPW generator_right;
    generator_left.SetFrequency(kFrequency);

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
    IsContinuous<SawtoothDPW> is_continuous(kMaxDelta,
                                            current_phase);
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
TEST(Generators, SawtoothDPWBeginsAtZero) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // Generating data
    SawtoothDPW generator;
    generator.SetFrequency(kFrequency);
    const float first_sample(GetFirst(generator()));
    EXPECT_EQ(0.0f, first_sample);
  }  // iterations?
}

/// @brief Generates a signal (performance tests)
TEST(Generators, SawtoothDPWPerf) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(kFreqDistribution(kRandomGenerator));
    SawtoothDPW generator;
    generator.SetFrequency(kFrequency);

    unsigned int sample_idx(0);
    while (sample_idx < kGeneratorDataPerfSetSize) {
      const Sample kCurrent(generator());
      sample_idx += soundtailor::SampleSize;
      // No actual test!
      EXPECT_TRUE(LessEqual(-2.0f, kCurrent));
    }
  }
}
