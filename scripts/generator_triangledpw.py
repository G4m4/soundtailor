#!/usr/bin/env python
'''
@file generator_triangledpw.py
@brief Triangle Signal generator based on DPW algorithm
@author gm
@copyright gm 2014

This file is part of SoundTailor

SoundTailor is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SoundTailor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SoundTailor.  If not, see <http://www.gnu.org/licenses/>.
'''

'''
Note that all of this mimics C++ code for testing/prototyping purpose.
Hence it may not really seems "pythonic" and not intended to be in any way.
'''

import math
from generators_common import GeneratorInterface, Differentiator, PhaseAccumulator

class TriangleDPW(GeneratorInterface):
    '''
    Implements a triangle signal generator based on DPW algorithm
    '''
    def __init__(self, sampling_rate):
        super(TriangleDPW, self).__init__(sampling_rate)
        self._sawtooth_gen = PhaseAccumulator(sampling_rate)
        self._differentiator = Differentiator()
        self._normalization_factor = 0.0
        self._frequency = 0.0
        self._update = False
        self.SetPhase(0.0)
        self.ProcessSample()

    def SetPhase(self, phase):
        value = phase * -0.5 + 0.5
        self._sawtooth_gen.SetPhase(value)

    def SetFrequency(self, frequency):
        self._frequency = frequency
        self._sawtooth_gen.SetFrequency(frequency)
        self._update = True

    def ProcessSample(self):
        self._ProcessParameters()
        current = self._sawtooth_gen.ProcessSample()
        current -= current * abs(current)
        current = self._differentiator.ProcessSample(current)
        current *= self._normalization_factor

        return current

    def _ProcessParameters(self):
        if self._update:
            self._normalization_factor = self._sampling_rate \
                                            / (2.0 * self._frequency)
            self._update = False

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import numpy
    import pylab
    import utilities

    sampling_freq = 96000
    freq = 2500.0
    length = 256

    # Change phase
    generated_data = numpy.zeros(length)
    internal_saw_data = numpy.zeros(length)
    internal_diff_data = numpy.zeros(length)
    ref_data = numpy.zeros(length)

    generator_ref = TriangleDPW(sampling_freq)
    generator_ref.SetFrequency(freq)
    for idx in range(length):
        ref_data[idx] = generator_ref.ProcessSample()

    generator_left = TriangleDPW(sampling_freq)
    generator_left.SetFrequency(freq)
    for idx in range(length / 2):
        generated_data[idx] = generator_left.ProcessSample()
        internal_saw_data[idx] = generator_left._sawtooth_gen._current
        internal_diff_data[idx] = generator_left._differentiator._last

    generator_right = TriangleDPW(sampling_freq)
    generator_right.SetPhase(generated_data[length / 2 - 1])
    generator_right.SetFrequency(freq)
    generator_right.ProcessSample()
    for idx in range(length / 2, length):
        generated_data[idx] = generator_right.ProcessSample()
        internal_saw_data[idx] = generator_right._sawtooth_gen._current
        internal_diff_data[idx] = generator_right._differentiator._last

    differentiator = Differentiator()
    diff_data = numpy.zeros(len(generated_data))
    for idx, sample in enumerate(generated_data):
        diff_data[idx] = differentiator.ProcessSample(sample)

    print(utilities.PrintMetadata(utilities.GetMetadata(generated_data - ref_data)))

    pylab.plot(generated_data, label = "generated")
    pylab.plot(internal_saw_data, label = "internal_saw")
    pylab.plot(internal_diff_data, label = "internal_diff")
    pylab.plot(diff_data, label = "diff")

    pylab.legend()
    pylab.show()

    utilities.WriteWav(generated_data, "triangle_gen", sampling_freq)
