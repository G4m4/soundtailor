#!/usr/bin/env python
"""
@file bandlimited_impulse.py
@brief Bandlimited impulse generation
@author gm
@copyright gm 2016

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
"""

'''
Note that all of this mimics C++ code for testing/prototyping purpose.
Hence it may not really seems "pythonic" and not intended to be in any way.
'''

import numpy


def GenerateBLI(sampling_rate = 48000.0,
                cutoff = 15000.0,
                length = 4,
                ppiv = 2700,
                beta = 8.3,
                apodization_factor = 0.5,
                apodization_beta = 0.5):
    """
    Generates a bandlimited impulse
    """
    kEpsilon = 1e-7
    points_count = ppiv * length
    xaxis = numpy.linspace(0.0, points_count, points_count)
    x2 = length * 2.0 * (xaxis - (points_count) / 2.0 + kEpsilon) / (points_count)
    x3 = numpy.pi * cutoff / sampling_rate * x2

    brickwall_impulse = numpy.sin(x3) / x3

    kaiser_window = numpy.kaiser(points_count, beta)
    bli_data = numpy.transpose(brickwall_impulse) * kaiser_window

    # Apodization
    apodization_window = (1.0 - apodization_factor) \
                            * numpy.kaiser(points_count,apodization_beta)
    bli_data *= apodization_window

    return bli_data


def GenerateBLSawtoothSegment(sampling_rate = 48000.0,
                              cutoff = 15000.0,
                              length = 4,
                              ppiv = 2700,
                              beta = 8.3,
                              apodization_factor = 0.5,
                              apodization_beta = 0.5):
    """
    Generates a bandlimited sawtooth segment
    Beware: generates the left half segment only due to symmetry!
    """
    saw_data = GenerateBLI(sampling_rate,
                           cutoff,
                           length,
                           ppiv,
                           beta,
                           apodization_factor,
                           apodization_beta)
    points_count = len(saw_data)

    # Cumulative sum & normalization
    saw_data = numpy.cumsum(saw_data)
    saw_data = 2.0 * saw_data / saw_data[points_count - 1]
    saw_data[numpy.floor(points_count / 2):] = saw_data[numpy.floor(points_count / 2):] - 2.0
    saw_data /= numpy.max(saw_data)

    return saw_data[0:points_count / 2]


def GenerateBLSawtoothIntegrate(sampling_rate = 48000.0,
                                cutoff = 15000.0,
                                length = 4,
                                ppiv = 256,
                                beta = 8.3,
                                apodization_factor = 0.5,
                                apodization_beta = 0.5):
    """
    Generates the integration of a bandlimited sawtooth segment
    Beware: generates the left half segment only due to symmetry!
    """
    out = GenerateBLSawtoothSegment(sampling_rate, cutoff, length, ppiv, beta, apodization_factor, apodization_beta)
    out = numpy.cumsum(out)
    out /= numpy.max(out)

    return out


class BLPostFilter(object):
    """
    High frequency booster filter for band limited generators
    """
    def __init__(self):
        self._last = 1.0

    def process_sample(self, sample):
        out = sample - 0.35 * self._last
        # out *= (3.0 / 10.0)
        self._last = out

        return out

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import pylab
    from utilities import write_c_array

    view_beginning = 0
    view_length = 4 * 2700

    bli_data = GenerateBLI()
    blsawtooth_segment_data = GenerateBLSawtoothSegment()
    blsawtooth_integrate_data = GenerateBLSawtoothIntegrate()
    pylab.plot(bli_data[view_beginning:view_beginning + view_length], label="blimpulse")
    pylab.plot(blsawtooth_segment_data[view_beginning:view_beginning + view_length], label="blsaw")
    pylab.plot(blsawtooth_integrate_data[view_beginning:view_beginning + view_length], label="blsaw_integration")

    write_c_array(blsawtooth_segment_data, "../soundtailor/src/generators/blsawtooth_segment.inc")
    write_c_array(blsawtooth_integrate_data, "../soundtailor/src/generators/blsawtooth_integration.inc")

    # length = 120
    # post_filter = BLPostFilter()
    # test_data = numpy.zeros(length)
    # in_data = 2.0 * numpy.random.rand(length) - 1.0
    # for idx in range(length):
    #     value = in_data[idx]
    #     test_data[idx] = post_filter.process_sample(value)
    # pylab.plot(in_data, label="in")
    # pylab.plot(test_data, label="out")

    pylab.legend()
    pylab.show()
