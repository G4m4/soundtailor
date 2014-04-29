#!/usr/bin/env python
'''
@file generator_sin.py
@brief Sinus signal generator sandbox
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

from generators_common import GeneratorInterface

class SinGenerator(GeneratorInterface):
    '''
    Simple sin synthesis
    '''
    def __init__(self, sampling_rate):
        super(SinGenerator, self).__init__(sampling_rate)
        self._phase_increment = 0.0
        self._phase_current = 0.0
        self._frequency = 0.0

    def SetPhase(self, phase):
        self._phase_current = math.acos(phase)

    def SetFrequency(self, value):
        self._frequency = value
        self._update = True

    def ProcessSample(self):
        self._ProcessParameters()
        out = math.cos(self._phase_current)
        # TODO(gm): beware the overflow case!
        self._phase_current += self._phase_increment

        return out

    def _ProcessParameters(self):
        if self._update:
            self._phase_increment = 2 * math.pi * (self._frequency / self._sampling_rate)
            self._update = False

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import numpy
    import pylab

    import generators_common
    import utilities

    freq = 1000.0
    sampling_freq = 48000
    length = 256

    generator = SinGenerator(sampling_freq)
    generator.SetFrequency(freq)
    generated_data = numpy.zeros(length)
    for idx, _ in enumerate(generated_data):
        generated_data[idx] = generator.ProcessSample()

    # Change phase
    generated_data = numpy.zeros(length)

    generator_left = SinGenerator(sampling_freq)
    generator_left.SetFrequency(freq)
    for idx in range(length / 2):
        generated_data[idx] = generator_left.ProcessSample()

    generator_right = SinGenerator(sampling_freq)
    generator_right.SetPhase(generated_data[length / 2 - 1])
    generator_right.SetFrequency(freq)
    generator_right.ProcessSample()
    for idx in range(length / 2, length):
        generated_data[idx] = generator_right.ProcessSample()

    differentiator = generators_common.Differentiator()
    diff_data = numpy.zeros(len(generated_data))
    for idx, sample in enumerate(generated_data):
        diff_data[idx] = differentiator.ProcessSample(sample)

    view_beginning = 0
    view_length = 512

    pylab.plot(generated_data[view_beginning:view_beginning + view_length], label = "generated")
    pylab.plot(diff_data[view_beginning:view_beginning + view_length], label = "diff")

    pylab.legend()
    pylab.show()
