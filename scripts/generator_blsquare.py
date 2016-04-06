#!/usr/bin/env python
"""
@file generator_blsquare.py
@brief Bandlimited square generation
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

from bandlimited_impulse import BLPostFilter
from generator_blsawtooth import BLSawtooth
from generators_common import GeneratorInterface, IncrementAndWrap


class BLSquare(GeneratorInterface):
    """
    Implements a band limited square signal generator
    Basically a fixed width pulse generator allowing to explore specific optimizations and features
    """
    def __init__(self, sampling_rate, with_postfilter=False):
        super(BLSquare, self).__init__(sampling_rate)
        self._gen1 = BLSawtooth(sampling_rate, False)
        self._gen2 = BLSawtooth(sampling_rate, False)
        self.SetPhase(0.0)
        if with_postfilter:
            self._post_filter = BLPostFilter()

    def SetPhase(self, phase):
        self._gen1.SetPhase(phase)
        self._gen2.SetPhase(IncrementAndWrap(phase, 1.0))

    def SetFrequency(self, frequency):
        self._gen1.SetFrequency(frequency)
        self._gen2.SetFrequency(frequency)

    def ProcessSample(self):
        out1 = self._gen1.ProcessSample()
        out2 = self._gen2.ProcessSample()

        out = out1 - out2

        if hasattr(self, '_post_filter'):
            return self._post_filter.process_sample(out)
        else:
            return out

if __name__ == "__main__":
    import numpy
    import pylab
    from utilities import GetPredictedLength, GenerateSquareData, GetMetadata, PrintMetadata, WriteWav

    sampling_freq = 48000
    # Prime, as close as possible to the upper bound of 4kHz
    freq = 3989.0
    length = GetPredictedLength(freq / sampling_freq, 3)
    length = 120

    # Change phase
    generated_data = numpy.zeros(length)
    ref_data = numpy.zeros(length)
    # -1 factor to start at 0
    base_data = -1.0 * GenerateSquareData(freq, length, sampling_freq)
    nopostfilter_data = numpy.zeros(length)

    generator_ref = BLSquare(sampling_freq)
    generator_ref.SetFrequency(freq)
    generator_nopostfilter = BLSquare(sampling_freq, False)
    generator_nopostfilter.SetFrequency(freq)
    for idx in range(length):
        ref_data[idx] = generator_ref.ProcessSample()
        nopostfilter_data[idx] = generator_nopostfilter.ProcessSample()

    generator_left = BLSquare(sampling_freq)
    generator_left.SetFrequency(freq)
    for idx in range(length / 2):
        generated_data[idx] = generator_left.ProcessSample()

    generator_right = BLSquare(sampling_freq)
    generator_right.SetFrequency(freq)
    generator_right.SetPhase(generated_data[length / 2 - 1])
    for idx in range(length / 2, length):
        generated_data[idx] = generator_right.ProcessSample()

    print(PrintMetadata(GetMetadata(ref_data)))

    # pylab.plot(generator_ref._table, label = "table")
    pylab.plot(ref_data, label = "square")
    pylab.plot(base_data, label = "square_base")
    pylab.plot(generated_data, label = "pieces_data")
    # pylab.plot(nopostfilter_data, label="square_nopf")

    pylab.legend()
    pylab.show()

    WriteWav(ref_data, "bl_square", sampling_freq)
