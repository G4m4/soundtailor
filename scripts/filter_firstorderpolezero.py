#!/usr/bin/env python
'''
@file filter_firstorderpolezero.py
@brief Simple prototype of a 1st order pole-zero lowpass filter
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

class PoleZeroLowPass(filters_common.FilterInterface):
    '''
    Implements a simple 1 pole Low pass
    '''
    def __init__(self):
        self._gain = 0.0
        self._coeff = 0.0
        self._last = 0.0

    def SetParameters(self, frequency, resonance):
        '''
        Sets frequency (normalized, < 0.5)
        '''
        b = math.pi * frequency
        self._coeff = (2.0 * math.sin(b)) / (math.cos(b) + math.sin(b))

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        direct = self._coeff / 2.0 * sample
        out = direct + self._last

        self._last = out * (1.0 - self._coeff) + direct

        return out

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import generator_sawtoothdpw
    import numpy
    import pylab
    import utilities

    freq = 110.0
    sampling_freq = 48000.0
    length = 1024
    filter_freq = 2400.0 / sampling_freq

    in_data = utilities.GenerateData(200, 2000, length, sampling_freq)
    generator = generator_sawtoothdpw.SawtoothDPW(sampling_freq)
    generator.SetFrequency(freq)
    for idx, _ in enumerate(in_data):
        in_data[idx] = generator.ProcessSample()

    out_data = numpy.zeros(length)

    lowpass = PoleZeroLowPass()
    lowpass.SetParameters(filter_freq, 0.0)

    for idx, sample in enumerate(in_data):
        out_data[idx] = lowpass.ProcessSample(sample)

    pylab.plot(in_data, label="in")
    pylab.plot(out_data, label="out")
    pylab.legend()
    pylab.show()
