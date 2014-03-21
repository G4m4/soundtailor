/// @file tests_triangle_dpw.cc
/// @brief Triangle DPW generator specific tests
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

#include "soundtailor/src/generators/triangle_dpw.h"

// Using declarations for tested generator
using soundtailor::generators::TriangleDPW;

/// @brief Generates a signal, check for null mean (no DC offset)
TEST(Generators, TriangleDPWMean) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // We are generating complete periods to prevent false positive
    const unsigned int kDataLength(static_cast<unsigned int>(
                                     std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriodsCount)));

    // Generating data
    TriangleDPW generator;
    generator.SetFrequency(kFrequency);

    const float kExpected(0.0f);
    const float kEpsilon(1e-2f);
    const float kActual(ComputeMean(generator, kDataLength));

    std::cout << "Frequency: " << kFrequency
              << "    Mean:" << kActual << std::endl;

    EXPECT_NEAR(kExpected, kActual, kEpsilon);
  }  // iterations?
}

/// @brief Generates a signal, check for signal power
TEST(Generators, TriangleDPWPower) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // We are generating complete periods to prevent false positive
    const unsigned int kDataLength(static_cast<unsigned int>(
                                     std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriodsCount)));

    // Generating data
    TriangleDPW generator;
    generator.SetFrequency(kFrequency);

    const float kExpected(1.0f / 3.0f);
    const float kEpsilon(2e-2f);
    const float kActual(ComputePower(generator, kDataLength));

    std::cout << "Frequency: " << kFrequency
              << "    Power:" << kActual << std::endl;

    EXPECT_NEAR(kExpected, kActual, kEpsilon);
  }  // iterations?
}

/// @brief Generates a signal,
/// check for normalized range (within [-1.0f ; 1.0f])
TEST(Generators, TriangleDPWRange) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

  const float kFrequency(kFreqDistribution(kRandomGenerator));

    TriangleDPW generator;
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
TEST(Generators, TriangleDPWZeroCrossings) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(kFreqDistribution(kRandomGenerator));

    const unsigned int kDataLength(static_cast<unsigned int>(
                                     std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriodsCount)));
    TriangleDPW generator;
    generator.SetFrequency(kFrequency);

    // Due to rounding one or even two zero crossings may be lost/added
    const int kEpsilon(2);
    const int kActual(ComputeZeroCrossing(generator, kDataLength));

    EXPECT_NEAR(kSignalDataPeriodsCount, kActual, kEpsilon);
  }
}

/// @brief Generates a signal at each frequency corresponding
/// to key notes from C1 to C8, check for expected zero crossing
TEST(Generators, TriangleDPWNotes) {
  for (unsigned int key_note(kMinKeyNote);
       key_note < kMaxKeyNote;
       ++key_note) {
    const float kFrequency(NoteToFrequency(key_note));
    const unsigned int kDataLength(
       static_cast<unsigned int>(std::floor((0.5f / kFrequency)
                                            * kSignalDataPeriodsCount
                                            * kSamplingRate)));
    TriangleDPW generator;
    generator.SetFrequency(kFrequency / kSamplingRate);

    // Due to rounding one or even two zero crossings may be lost/added
    const int kEpsilon(2);
    const int kActual(ComputeZeroCrossing(generator, kDataLength));

    EXPECT_NEAR(kSignalDataPeriodsCount, kActual, kEpsilon);
  }
}

/// @brief Generate half a signal with a generator, use another generator
/// for the other half by forcing the second generator phase.
/// No difference should be perceptible with single-generator data
TEST(Generators, TriangleDPWPhaseControl) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // Arbitrary data length, preferably a small one to reduce test duration
    const unsigned int kDataLength(2048);

    // Generating data
    TriangleDPW generator_ref;
    TriangleDPW generator_left;
    TriangleDPW generator_right;
    generator_ref.SetFrequency(kFrequency);
    generator_left.SetFrequency(kFrequency);
    generator_right.SetFrequency(kFrequency);

    const float kEpsilon(1e-6f);

    // Creating an history
    Sample sample(Fill(0.0f));
    for (unsigned int i(0);
         i < kDataLength / 2;
         i += soundtailor::SampleSize) {
      sample = generator_left();
      const Sample ref(generator_ref());
      EXPECT_TRUE(IsNear(sample, ref, kEpsilon));
    }
    // Forcing right generator phase
    generator_right.SetPhase(GetLast(sample));

    // Now using right generator
    for (unsigned int i(kDataLength / 2);
         i < kDataLength;
         i += soundtailor::SampleSize) {
      sample = generator_right();
      const Sample ref(generator_ref());
      EXPECT_TRUE(IsNear(sample, ref, kEpsilon));
    }
  }  // iterations?
}

/// @brief Generates a signal (performance tests)
TEST(Generators, TriangleDPWPerf) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(kFreqDistribution(kRandomGenerator));
    TriangleDPW generator;
    generator.SetFrequency(kFrequency);

    unsigned int sample_idx(0);
    while (sample_idx < kGeneratorDataPerfSetSize) {
      const Sample kCurrent(generator());
      sample_idx += soundtailor::SampleSize;
      // No actual test!
      EXPECT_TRUE(LessEqual(-1.0f, kCurrent));
    }
  }
}
