#!/usr/bin/env python
'''
@file filter_oversampler.py
@brief Simple prototype for naive filter oversampling
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

import filters_common

class Oversampler(filters_common.FilterInterface):
    '''
    Oversample a filter by repeated calls to its process function
    '''
    def __init__(self, filter_class):
        self._filter = filter_class()

    def SetParameters(self, frequency, resonance):
        '''
        Everything is forwarded to the actual filter instance
        '''
        self._filter.SetParameters(frequency, resonance)

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        self._filter.ProcessSample(sample)

        return self._filter.ProcessSample(sample)

    def Process4Samples(self, sample):
        '''
        Actual process function
        '''
        self._filter.Process4Samples(sample)

        return self._filter.Process4Samples(sample)

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import numpy
    import pylab
    import utilities

    import filter_secondorderraw

    freq = 24000.0
    sampling_freq = 48000.0
    length = 512
    resonance = 0.7

    in_data = utilities.GenerateChirpData(200, 4000, length, sampling_freq)
    in_data = 2.0 * numpy.random.rand(length) - 1.0

    out_data = numpy.zeros(length)
    out_data_ref = numpy.zeros(length)
    out_data_vectorized = numpy.zeros(length)

    lowpass = Oversampler(filter_secondorderraw.SecondOrderRaw)
    lowpass.SetParameters(freq / sampling_freq, resonance)

    for idx, _ in enumerate(in_data):
        out_data[idx] = lowpass.ProcessSample(in_data[idx])

    # Comparison with non-oversampled version
    lowpass_ref = filter_secondorderraw.SecondOrderRaw()
    lowpass_ref.SetParameters(freq / sampling_freq, resonance)

    for idx, _ in enumerate(in_data):
        out_data_ref[idx] = lowpass_ref.ProcessSample(in_data[idx])

    # Vectorized processing
    lowpass_v = Oversampler(filter_secondorderraw.SecondOrderRaw)
    lowpass_v.SetParameters(freq / sampling_freq, resonance)
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

    print (str(numpy.sum(out_data - out_data_ref)))
    pylab.plot(in_data, label="in")
    pylab.plot(out_data, label="out")
    pylab.plot(out_data_ref, label="ref")
    pylab.plot(out_data_vectorized, label="out_v")
#     pylab.plot(out_data - out_data_ref, label="diff")
    pylab.legend()
    pylab.show()
