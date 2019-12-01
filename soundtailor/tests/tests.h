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
#include <iostream>
#include <random>

#include "gtest/gtest.h"

#include "soundtailor/src/common.h"
#include "soundtailor/src/configuration.h"
#include "soundtailor/src/maths.h"
#include "soundtailor/src/utilities.h"

#include "soundtailor/tests/analysis.h"

using soundtailor::IGNORE;

// Using declarations for soundtailor maths stuff
using soundtailor::Sample;
using soundtailor::SampleRead;
using soundtailor::VectorMath;

using soundtailor::ComputeMean;
using soundtailor::ComputePower;
using soundtailor::ComputeZeroCrossing;
using soundtailor::IsContinuous;
using soundtailor::ZeroCrossing;

// Common base random distributions
static std::uniform_real_distribution<float> kNormDistribution(-1.0f, 1.0f);
static std::uniform_real_distribution<float> kNormPosDistribution(0.0f, 1.0f);
static std::bernoulli_distribution kBoolDistribution;

/// @brief: Basic helper
inline unsigned GetMultipleOf4(const unsigned value) {
  return value - (value % 4);
}

#endif  // SOUNDTAILOR_TESTS_TESTS_H_
