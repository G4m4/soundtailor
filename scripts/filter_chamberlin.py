#!/usr/bin/env python
'''
@file filter_chamberlin.py
@brief Prototype of a Chamberlin state variable filter
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

import filters_common

class Chamberlin(filters_common.FilterInterface):
    '''
    Implements a Chamberlin state variable filter
    '''
    def __init__(self):
        self._frequency = 0.0
        self._damping = 0.0
        self._hp = 0.0
        self._bp = 0.0
        self._lp = 0.0

    def SetParameters(self, frequency, resonance):
        '''
        Sets both frequency and resonance
        '''
        self._frequency = 2.0 * math.sin(math.pi * frequency)
        self._damping = 1.0 / resonance

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        self._lp = self._frequency * self._bp + self._lp
        self._hp = sample - self._lp - self._bp * self._damping
        self._bp = self._frequency * self._hp + self._bp

        return self._lp

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import numpy
    import pylab
    import utilities

    import generator_sawtoothdpw

    freq = 1000.0
    sampling_freq = 48000.0
    length = 256
    filter_freq = 1.0 / 6.0
    resonance = 1.0

    in_data = utilities.GenerateData(100, 2000, length, sampling_freq)
    generator = generator_sawtoothdpw.SawtoothDPW(sampling_freq)
    generator.SetFrequency(freq)
    for idx, _ in enumerate(in_data):
        in_data[idx] = generator.ProcessSample()

    in_data = numpy.random.rand(length)
    out_data = numpy.zeros(length)

    lowpass = Chamberlin()
    lowpass.SetParameters(filter_freq, resonance)

    for idx, _ in enumerate(in_data):
        out_data[idx] = lowpass.ProcessSample(in_data[idx])

    pylab.plot(in_data, label="in")
    pylab.plot(out_data, label="out")
    pylab.legend()
    pylab.show()
