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
        self._last_input = 0.0
        self._last_output = 0.0

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
        direct = sample + self._last_input
        direct *= self._coeff / 2.0
        out = direct + self._last_output * (1.0 - self._coeff)

        self._last_input = sample
        self._last_output = out

        return out

    def Process4Samples(self, vector):
        '''
        Actual process function - vectorized version
        '''
        direct_v = numpy.add(vector, filters_common.RotateOnRight(vector, self._last_input))
        direct_v = filters_common.MulConst(direct_v, self._coeff / 2.0)
        oldest_out = direct_v[0] + self._last_output * (1.0 - self._coeff)
        old_out = direct_v[1] + oldest_out * (1.0 - self._coeff)
        new_out = direct_v[2] + old_out * (1.0 - self._coeff)
        newest_out = direct_v[3] + new_out * (1.0 - self._coeff)

        out = (oldest_out, old_out, new_out, newest_out)

        self._last_input = vector[3]
        self._last_output = out[3]

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
    out_data_vectorized = numpy.zeros(length)

    lowpass = PoleZeroLowPass()
    lowpass.SetParameters(filter_freq, 0.0)

    for idx, sample in enumerate(in_data):
        out_data[idx] = lowpass.ProcessSample(sample)

    # Vectorized processing
    lowpass_v = PoleZeroLowPass()
    lowpass_v.SetParameters(filter_freq, 0.0)
    idx = 0
    while idx < len(in_data):
        current_vector = (in_data[idx],
                          in_data[idx + 1],
                          in_data[idx + 2],
                          in_data[idx + 3])
        (out_data_vectorized[idx],
         out_data_vectorized[idx + 1],
         out_data_vectorized[idx + 2],
         out_data_vectorized[idx + 3]) = lowpass_v.Process4Samples(current_vector)
        idx += 4

    print(utilities.PrintMetadata(utilities.GetMetadata(out_data - out_data_vectorized)))

    pylab.plot(in_data, label="in")
    pylab.plot(out_data, label="out")
    pylab.plot(out_data_vectorized, label="out_vectorized")
    pylab.legend()
    pylab.show()
