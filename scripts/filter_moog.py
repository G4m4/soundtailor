#!/usr/bin/env python
'''
@file filter_moog.py
@brief Prototype of a Moog filter
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
import filter_firstorderpolezero

class MoogBaseLowpass(filter_firstorderpolezero.FixedPoleZeroLowPass):
    '''
    Implements a simple 1 pole - 1 zero Low pass, with fixed coeffs tuned
    in order to be part of a bigger Moog filter
    '''
    def __init__(self, zero_coeff):
        # The pole coeff will change, only the zero one is fixed here
        super(MoogBaseLowpass, self).__init__(zero_coeff)

    def SetParameters(self, frequency, resonance):
        '''
        Sets frequency (normalized, < 1.0) and resonance (0 <= resonance < 4.0)
        '''
        f = frequency * (1.0 + 0.03617 * frequency
                                        * (4.0 - resonance) * (4.0 - resonance))
        self._big_f = 1.25 * f * (1.0 - 0.595 * f + 0.24 * f * f)
        self._big_r = resonance * (1.0 + 0.077 * self._big_f
                                     - 0.117 * self._big_f * self._big_f
                                     - 0.049 * self._big_f * self._big_f * self._big_f)
        super(MoogBaseLowpass, self).SetParameters(self._big_f, self._big_r)

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        direct = self._pole_coeff / 2.0 * sample
        out = direct + self._last

        self._last = out * (1.0 - self._pole_coeff) + self._zero_coeff * direct

        return out

class Moog(filters_common.FilterInterface):
    '''
    Implements a Moog filter
    '''
    def __init__(self):
        self._frequency = 0.0
        self._damping = 0.0

    def SetParameters(self, frequency, resonance):
        '''
        Sets both frequency and resonance
        '''
        pass

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        pass

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
    filter_freq = 1.0
    resonance = 0.1

    in_data = utilities.GenerateData(100, 2000, length, sampling_freq)
    generator = generator_sawtoothdpw.SawtoothDPW(sampling_freq)
    generator.SetFrequency(freq)
    for idx, _ in enumerate(in_data):
        in_data[idx] = generator.ProcessSample()

    in_data = numpy.random.rand(length) * 2.0 - 1.0
    out_data = numpy.zeros(length)
    out_base_lowpass = numpy.zeros(length)

    base_lowpass = MoogBaseLowpass(0.3)
    base_lowpass.SetParameters(filter_freq, resonance)
    lowpass = Moog()
    lowpass.SetParameters(filter_freq, resonance)

    for idx, _ in enumerate(in_data):
        out_data[idx] = lowpass.ProcessSample(in_data[idx])
        out_base_lowpass[idx] = base_lowpass.ProcessSample(in_data[idx])

    pylab.plot(in_data, label="in")
    pylab.plot(out_base_lowpass, label="out_baselp")
    pylab.plot(out_data, label="out")
    pylab.legend()
    pylab.show()
