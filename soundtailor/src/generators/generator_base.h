/// @file generator_base.h
/// @brief SoundTailor generators base class declaration
///
/// Base interface and common methods for all generators
///
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

#ifndef SOUNDTAILOR_SRC_GENERATORS_GENERATOR_BASE_H_
#define SOUNDTAILOR_SRC_GENERATORS_GENERATOR_BASE_H_

#include "soundtailor/src/common.h"
#include "soundtailor/src/maths.h"

namespace soundtailor {
namespace generators {

/// @brief Base class, defining common methods
/// to be implemented in all generators
///
/// All generators are "dumb" - they are responsible for generating sound,
/// and nothing else: they do not do any fancy parameter management.
/// It means that any asynchronous parameter update has to be managed upstream,
/// because the setters here are "instantaneous".
class Generator_Base {
 public:
  /// @brief Default constructor - generator phase may be defined here,
  /// allowing gapless instantiation from another generator
  ///
  /// @param[in]  phase   Phase to initialize the generator to
  explicit Generator_Base(const float phase = 0.0f) {
    // Nothing to do here for now
    IGNORE(phase);
  }

  virtual ~Generator_Base() {
    // Nothing to do here for now
  }

  /// @brief Actual process function for one sample
  ///
  /// Process is done per-sample for flexibility purpose, beware of not having
  /// one of this called per-sample! Check in the final code that it actually
  /// gets inlined if needed
  virtual Sample operator()(void) = 0;

  /// @brief Reset the instance to the given phase - nothing else gets changed
  ///
  /// Phase is normalized - the input value should be in [-1.0f ; 1.0f]
  ///
  /// @param[in]  phase   Phase to set the generator to
  virtual void SetPhase(const float phase) = 0;

  /// @brief Set the generator to the given output frequency
  ///
  /// Frequency is normalized - the input value should be in [0.0f ; 0.5f]
  /// 0.5 because:
  /// f_{n} = \frac{f}{f_{s}}
  /// and:
  /// f_{max} = \frac{f_{s}}{2}
  ///
  /// @param[in]  frequency   Frequency to set the generator to
  virtual void SetFrequency(const float frequency) = 0;

  /// @brief Make the generator take new parameters into account
  ///
  /// This is mandatory after a call to SetPhase(), SetFrequency(), or both
  ///
  /// Not having this method automated is chosen in order to keep those
  /// generators very lightweight
  ///
  /// @return generated sample
  virtual float ProcessParameters(void) = 0;
};

/// @brief Actual process function for many samples
///
/// In a context of dynamic polymorphism this will save you from per-sample
/// virtual function calls
/// The compiler should be able to inline it
/// obviously the instance has to be known at compile time
template <typename GeneratorType>
void ProcessBlock(BlockOut out,
                  unsigned int block_size,
                  GeneratorType&& instance) {
  float* out_write(out);
  for (unsigned int i(0); i < block_size; i += SampleSize) {
    VectorMath::Store(out_write, instance());
    out_write += SampleSize;
  }
}

}  // namespace generators
}  // namespace soundtailor

#endif  // SOUNDTAILOR_SRC_GENERATORS_GENERATOR_BASE_H_
