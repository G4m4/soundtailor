/// @file tests_generators_common.cc
/// @brief SoundTailor common generators tests
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

#include "soundtailor/src/generators/generators_common.h"

// Using declarations for tested generator
using soundtailor::generators::PhaseAccumulator;
using soundtailor::generators::Differentiator;

/// @brief Generates a signal, check for null mean (no DC offset)
TEST(Generators, PhaseAccumulatorMean) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);
    // Random normalized frequency
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // We are generating complete periods to prevent false positive
    const unsigned int kDataLength(static_cast<unsigned int>(
                                     std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriodsCount)));

    // Generating data
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    const float kExpected(0.0f);
    // Epsilon is quite big here, this generator being very crude
    const float kEpsilon(1e-1f);
    const float kActual(ComputeMean(generator, kDataLength));

    std::cout << "Frequency: " << kFrequency
              << "    Mean:" << kActual << std::endl;

    EXPECT_NEAR(kExpected, kActual, kEpsilon);
  }  // iterations?
}

/// @brief Generates a signal, check for signal power
TEST(Generators, PhaseAccumulatorPower) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    // Random normalized frequency
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // We are generating complete periods to prevent false positive
    const unsigned int kDataLength(static_cast<unsigned int>(
                                     std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriodsCount)));

    // Generating data
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    const float kExpected(1.0f / 3.0f);
    // Very low epsilon with this algorithm!
    const float kEpsilon(6e-3f);
    const float kActual(ComputePower(generator, kDataLength));

    std::cout << "Frequency: " << kFrequency
              << "    Power:" << kActual << std::endl;

    EXPECT_NEAR(kExpected, kActual, kEpsilon);
  }  // iterations?
}

/// @brief Generates a signal,
/// check for normalized range (within [-1.0f ; 1.0f])
TEST(Generators, PhaseAccumulatorRange) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(kFreqDistribution(kRandomGenerator));
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    for (unsigned int i(0);
         i < kDataTestSetSize;
         i += soundtailor::SampleSize) {
      const Sample sample(Fill(kFreqDistribution(kRandomGenerator)));
      EXPECT_TRUE(GreaterEqual(1.0f, sample));
      EXPECT_TRUE(LessEqual(-1.0f, sample));
    }
  }
}

/// @brief Generates a signal and check for expected zero crossing
/// according parameterized frequency (1 expected zero crossings per period)
TEST(Generators, PhaseAccumulatorZeroCrossings) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(kFreqDistribution(kRandomGenerator));
    const unsigned int kDataLength(static_cast<unsigned int>(
                                     std::floor((0.5f / kFrequency)
                                                * kSignalDataPeriodsCount)));
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    // Due to rounding one or even two zero crossings may be lost/added
    const int kEpsilon(2);
    const int kActual(ComputeZeroCrossing(generator, kDataLength));

    EXPECT_NEAR(kSignalDataPeriodsCount, kActual, kEpsilon);
  }
}

/// @brief Generate half a signal with a generator, use another generator
/// for the other half by forcing the second generator phase.
/// No difference should be perceptible at the transition
TEST(Generators, PhaseAccumulatorPhaseControl) {
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
    // Forcing right generator phase
    const float current_phase(GetLast(sample));
    generator_right.SetPhase(current_phase);
    IsContinuous<PhaseAccumulator> is_continuous(generator_right,
                                                 kMaxDelta,
                                                 current_phase);
    // Check the next 4 Samples for continuity:
    // only the transition is interesting here
    for (unsigned int i(kHistoryLength);
         i < kHistoryLength + 4;
         i += soundtailor::SampleSize) {
           bool test(is_continuous());
      EXPECT_TRUE(test);
    }
  }  // iterations?
}

/// @brief Check that the first generated sample is always a zero
TEST(Generators, PhaseAccumulatorBeginsAtZero) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);
    const float kFrequency(kFreqDistribution(kRandomGenerator));

    // Generating data
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);
    const float first_sample(GetFirst(generator()));
    EXPECT_EQ(0.0f, first_sample);
  }  // iterations?
}

/// @brief Generates a signal (performance tests)
TEST(Generators, PhaseAccumulatorPerf) {
  for (unsigned int iterations(0); iterations < kIterations; ++iterations) {
    IGNORE(iterations);

    const float kFrequency(kFreqDistribution(kRandomGenerator));
    PhaseAccumulator generator;
    generator.SetFrequency(kFrequency);

    unsigned int sample_idx(0);
    while (sample_idx < kGeneratorDataPerfSetSize) {
      const Sample kCurrent(Fill(kFreqDistribution(kRandomGenerator)));
      sample_idx += soundtailor::SampleSize;
      // No actual test!
      EXPECT_TRUE(LessEqual(-2.0f, kCurrent));
    }
  }
}

/// @brief Differentiate a constant, check for null derivative
TEST(Generators, DifferentiatedConstant) {
  // The input is a random value in [-1.0f ; 1.0f]
  const Sample input(Fill(kNormDistribution(kRandomGenerator)));
  Differentiator differentiator;
  // Not checking the first value!
  differentiator(input);
  for (unsigned int i(soundtailor::SampleSize);
       i < kDataTestSetSize;
       i += soundtailor::SampleSize) {
    EXPECT_TRUE(Equal(0.0f, differentiator(input)));
  }
}

/// @brief Generates a triangle, check for its differentiated output:
/// it is supposed to be almost null everywhere except at discontinuities
TEST(Generators, DifferentiatedSawtooth) {
  const float kFrequency(kFreqDistribution(kRandomGenerator));
  PhaseAccumulator generator;
  generator.SetFrequency(kFrequency);

  // This is the sawtooth period e.g. each time the discontinuity occurs
  const int kPeriod(static_cast<int>(std::floor(kFrequency * kSamplingRate)));
  // The sawtooth is not perfect:
  // there may be a small DC offset for its derivative
  const float kThreshold(0.15f);

  Differentiator differentiator;
  for (unsigned int i(0); i < kDataTestSetSize; i += soundtailor::SampleSize) {
    const Sample input(generator());
    const Sample diff(differentiator(input));
    if (i % kPeriod != 0) {
      EXPECT_TRUE(GreaterThan(kThreshold, diff));
    }
  }
}

/// @brief Differentiate random values (performance test)
TEST(Generators, DifferentiatorPerf) {
  Differentiator differentiator;
  for (unsigned int i(0);
       i < kFilterDataPerfSetSize;
       i += soundtailor::SampleSize) {
    const Sample input(Fill(kNormDistribution(kRandomGenerator)));
    const Sample diff(differentiator(input));
    // No actual test!
    EXPECT_TRUE(LessThan(-2.0f, diff));
  }
}
