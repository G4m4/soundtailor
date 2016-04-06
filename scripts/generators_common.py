#!/usr/bin/env python
'''
@file generators_common.py
@brief Common definitions for all generators
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

class GeneratorInterface(object):
    '''
    Common interface for all generators - defines functions to be implemented
    by all subclasses (calling one of this "pure virtual" function will throw)
    '''
    def __init__(self, sampling_rate):
        '''
        The only difference with actual C++ code:
        here in Python we are initializing generators with a sampling rate,
        instead of using a static constant
        '''
        self._sampling_rate = sampling_rate

    def SetPhase(self, phase):
        raise Exception("'Pure virtual' function called!")

    def SetFrequency(self, frequency):
        raise Exception("'Pure virtual' function called!")

    def ProcessSample(self):
        raise Exception("'Pure virtual' function called!")

    def _ProcessParameters(self):
        raise Exception("'Pure virtual' function called!")


def IncrementAndWrap(input_sample, increment):
    """
    Helper function: increment the input and wraps it into [-1.0 ; 1.0[
    The input is supposed not to be negative
    """
    output = input_sample + increment
    if output > 1.0:
        output -= 2.0
    return output


class PhaseAccumulator(GeneratorInterface):
    """
    Implements a naive sawtooth generator by phase accumulation
    """
    def __init__(self, sampling_rate):
        super(PhaseAccumulator, self).__init__(sampling_rate)
        self._increment = 0.0
        self._current = 0.0
        self._frequency = 0.0
        self._update = False

    def SetPhase(self, phase):
        self._ProcessParameters()
        self._current = phase

    def SetFrequency(self, frequency):
        self._frequency = frequency
        self._update = True

    def ProcessSample(self):
        self._ProcessParameters()
        out = self._current
        self._current = IncrementAndWrap(self._current, self._increment)
        return out

    def _ProcessParameters(self):
        if self._update:
            self._increment = (2.0 * self._frequency) / self._sampling_rate
            self._update = False

class Differentiator(object):
    '''
    Implements a 1st-order differentiator the scalar way
    '''
    def __init__(self, last = 0.0):
        self._last = last

    def SetHistory(self, last):
        self._last = last

    def ProcessSample(self, sample):
        before_diff = sample
        after_diff = sample - self._last
        self._last = before_diff
        return after_diff

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import numpy
    import pylab
    from utilities import GenerateSawtoothData

    freq = 3989.0
    sampling_freq = 48000.0
    length = 64

    # Change phase
    ref_data = numpy.zeros(length)
    base_data = GenerateSawtoothData(freq, length, sampling_freq)
    generated_data = numpy.zeros(length)

    generator_ref = PhaseAccumulator(sampling_freq)
    generator_ref.SetFrequency(freq)
    generator_ref.SetPhase(-1.0)
    for idx in range(length):
        ref_data[idx] = generator_ref.ProcessSample()

    generator_left = PhaseAccumulator(sampling_freq)
    generator_left.SetFrequency(freq)
    generator_left.SetPhase(-1.0)
    for idx in range(length / 2):
        generated_data[idx] = generator_left.ProcessSample()

    generator_right = PhaseAccumulator(sampling_freq)
    generator_right.SetFrequency(freq)
    generator_right.SetPhase(generated_data[length / 2 - 1])
    generator_right.ProcessSample()
    for idx in range(length / 2, length):
        generated_data[idx] = generator_right.ProcessSample()

    differentiator = Differentiator()
    diff_data = numpy.zeros(len(generated_data))
    for idx, sample in enumerate(generated_data):
        diff_data[idx] = differentiator.ProcessSample(sample)

    pylab.plot(ref_data, label="ref")
    pylab.plot(base_data, label="base_data")
    pylab.plot(generated_data, label="pieces_data")
    pylab.plot(diff_data, label="diff")
    pylab.legend()
    pylab.show()