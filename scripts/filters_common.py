#!/usr/bin/env python
'''
@file filters_common.py
@brief Common definitions for all filters
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
import numpy

def VectorDotProduct(left, right):
    '''
    Explicit vectorized dot product between two vectors:
    given left = (x0, x1, x2, x3) and right = (y0, y1, y2, y3)
    it will return x0 * y0 + x1 * y1 + x2 * y2 + x3 * y3

    @param    left     4 samples tuple
    @param    right    4 samples tuple
    @return    One scalar value
    '''
    return left[0] * right[0] \
           + left[1] * right[1] \
           + left[2] * right[2] \
           + left[3] * right[3]

def VectorRotateEachHalf(vector, left, right):
    '''
    Rotate each half of the given vector, introducing given values.
    E.g. given vector = (x0, x1, x2, x3)
    it will return (x1, left, x3, right)

    @param    vector    4 samples tuple
    @param    left    value to shift in the left half
    @param    right    value to shift in the right half
    @return    4 samples tuple
    '''
    return (vector[1],
            left,
            vector[3],
            right)

def RotateOnLeft(vector, value):
    '''
    Rotate the given vector, introducing given value.
    E.g. given vector = (x0, x1, x2, x3)
    it will return (x1, x2, x3, value)

    @param    vector    4 samples tuple
    @param    value    value to shift in
    @return    4 samples tuple
    '''
    return (vector[1],
            vector[2],
            vector[3],
            value)

def RotateOnRight(vector, value):
    '''
    Rotate the given vector, introducing given value.
    E.g. given vector = (x0, x1, x2, x3)
    it will return (value, x0, x1, x2)

    @param    vector    4 samples tuple
    @param    value    value to shift in
    @return    4 samples tuple
    '''
    return (value,
            vector[0],
            vector[1],
            vector[2])

def TakeEachRightHalf(left, right):
    '''
    Get each right half of the two given vectors:
    given left = (x0, x1, x2, x3) and right = (y0, y1, y2, y3)
    it will return (x2, x3, y2, y3)

    @param    left     4 samples tuple
    @param    right    4 samples tuple
    @return    4 samples tuple
    '''
    return (left[2],
            left[3],
            right[2],
            right[3])

def Fill(value):
    '''
    Fill a 4 samples tuple with the given value

    @param    value    Value to fill with
    @return    4 samples tuple
    '''
    return (value,
            value,
            value,
            value)

def MulConst(vector, value):
    '''
    Multiplies each element of the given vector by "value"

    @param    vector    Vector to operate on
    @param    value    Value to fill with
    @return    4 samples tuple
    '''
    return numpy.multiply(value, vector)

class FilterInterface(object):
    '''
    Common interface for all filters - defines functions to be implemented
    by all subclasses (calling one of this "pure virtual" function will throw)
    '''
    def __init__(self):
        pass

    def SetParameters(self, frequency, resonance):
        raise Exception("'Pure virtual' function called!")

    def ProcessSample(self):
        raise Exception("'Pure virtual' function called!")

class AllPoleLowPass(FilterInterface):
    '''
    Implements a simple 1st order all-pole Low pass
    '''
    def __init__(self):
        self._gain = 0.0
        self._coeff = 0.0
        self._history = 0.0

    def SetParameters(self, frequency, resonance):
        '''
        Sets frequency (normalized, < 0.5)
        '''
        b = (math.sin(math.pi * frequency) * math.sin(math.pi * frequency))
        self._coeff = 2.0 * (math.sqrt(b * b + b) - b)

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        out = self._coeff * sample + (1.0 - self._coeff) * self._history
        self._history = out

        return out

class PoleZeroLowPass(FilterInterface):
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
        direct_v = numpy.add(vector, RotateOnRight(vector, self._last_input))
        direct_v = MulConst(direct_v, self._coeff / 2.0)
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
    out_data_allpole = numpy.zeros(length)
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

    lowpass_allpole = AllPoleLowPass()
    lowpass_allpole.SetParameters(filter_freq, 0.0)

    for idx, sample in enumerate(in_data):
        out_data_allpole[idx] = lowpass_allpole.ProcessSample(sample)

    pylab.plot(in_data, label="in")
    pylab.plot(out_data, label="out")
    pylab.plot(out_data_allpole, label="out_allpole")
    pylab.plot(out_data_vectorized, label="out_vectorized")
    pylab.legend()
    pylab.show()
