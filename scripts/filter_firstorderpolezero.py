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
    Implements a simple 1 pole - 1 zero Low pass
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

    def Process4Samples(self, vector):
        '''
        Actual process function - vectorized version
        '''
        direct_v = filters_common.MulConst(vector, self._coeff / 2.0)
        oldest_out = direct_v[0] \
                    + self._last
        old_out = direct_v[1] \
                    + oldest_out * (1.0 - self._coeff) + direct_v[0]
        new_out = direct_v[2] \
                    + old_out * (1.0 - self._coeff) + direct_v[1]
        newest_out = direct_v[3] \
                    + new_out * (1.0 - self._coeff) + direct_v[2]

        out = (oldest_out, old_out, new_out, newest_out)

        self._last = newest_out * (1.0 - self._coeff) + direct_v[3]

        return out

class FixedPoleZeroLowPass(PoleZeroLowPass):
    '''
    Implements a simple 1 pole - 1 zero Low pass, with a fixed zero coeff and
    the ability to initially tune it
    '''
    def __init__(self, zero_coeff):
        # The pole coeff will change, only the zero one is fixed here
        super(FixedPoleZeroLowPass, self).__init__()
        self._gain = 0.0
        self._pole_coeff = 0.0
        self._zero_coeff = zero_coeff
        self._last = 0.0

    def SetParameters(self, frequency, resonance):
        '''
        Sets frequency (normalized, < 0.5)
        '''
        b = math.pi * frequency
        self._pole_coeff = (2.0 * math.sin(b)) / (math.cos(b) + math.sin(b))

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        direct = self._pole_coeff / 2.0 * sample
        out = direct + self._last

        self._last = out * (1.0 - self._pole_coeff) + self._zero_coeff * direct

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
    out_fixed_data = numpy.zeros(length)
    out_data_vectorized = numpy.zeros(length)

    lowpass = PoleZeroLowPass()
    lowpass.SetParameters(filter_freq, 0.0)
    # Assigning the same coeff to the "fixed" version with unitary zero coeff
    # should gives the same output
    fixed_lowpass = FixedPoleZeroLowPass(1.0)
    fixed_lowpass.SetParameters(filter_freq, 0.0)

    for idx, sample in enumerate(in_data):
        out_data[idx] = lowpass.ProcessSample(sample)
        out_fixed_data[idx] = fixed_lowpass.ProcessSample(sample)

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

    print(utilities.PrintMetadata(utilities.GetMetadata(out_data - out_fixed_data)))

    pylab.plot(in_data, label="in")
    pylab.plot(out_data, label="out")
    pylab.plot(out_data_vectorized, label="out_vectorized")
    pylab.plot(out_fixed_data, label="out_fixed")
    pylab.legend()

    pylab.show()
