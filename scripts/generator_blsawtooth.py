#!/usr/bin/env python
"""
@file generator_blsawtooth.py
@brief Bandlimited sawtooth generation
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

from bandlimited_impulse import GenerateBLSawtoothSegment, BLPostFilter
from generators_common import GeneratorInterface, PhaseAccumulator, IncrementAndWrap


class BLSawtooth(GeneratorInterface):
    """
    Implements a sawtooth signal generator based on BLIT algorithm
    """
    def __init__(self, sampling_rate, with_postfilter=True):
        super(BLSawtooth, self).__init__(sampling_rate)
        self._table = GenerateBLSawtoothSegment(sampling_rate=sampling_rate, length = 4)
        self._halfM = len(self._table)
        self._sawtooth_gen = PhaseAccumulator(sampling_rate)
        self._frequency = 0.0
        self._update = False
        self._alpha = 1.0
        self._N = 1.0
        self.debug_A = 0.0
        self.debug_B = 0.0
        self.debug_C = 0.0
        self.SetPhase(1.0)
        self.ProcessSample()
        if with_postfilter:
            self._post_filter = BLPostFilter()

        print(self._halfM)

    def SetPhase(self, phase):
        self._sawtooth_gen.SetPhase(phase)

    def SetFrequency(self, frequency):
        self._frequency = frequency
        self._sawtooth_gen.SetFrequency(frequency)
        self._update = True

    def _read_table(self, value):
        abs_value = numpy.abs(value)
        sign_value = numpy.sign(value)
        if abs_value < self._alpha:
            relative_index = int(numpy.round(self._halfM * abs_value / self._alpha))
            index = numpy.minimum(self._halfM - relative_index, self._halfM - 1)
            read = sign_value * self._table[index]
            # print(str(value) + " " + str(relative_index) + " " + str(index) + "->" + str(read))
            return read
        else:
            return 0.0

    def ProcessSample(self):
        self._ProcessParameters()
        current = self._sawtooth_gen.ProcessSample()
        A = current
        C = self._read_table(A)
        B = IncrementAndWrap(A, self._N)
        out = B + C

        self.debug_A = A
        self.debug_B = B
        self.debug_C = C

        if hasattr(self, '_post_filter'):
            return self._post_filter.process_sample(out)
        else:
            return out

    def _ProcessParameters(self):
        if self._update:
            self._alpha = self._frequency * 4.0 * self._N / self._sampling_rate
            print(self._alpha)
            self._update = False

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import numpy
    import pylab
    import utilities
    from scipy.signal import freqz

    sampling_freq = 48000
    # Prime, as close as possible to the upper bound of 4kHz
    freq = 3989.0
    length = 12000

    # Change phase
    generated_data = numpy.zeros(length)
    ref_data = numpy.zeros(length)
    nopostfilter_data = numpy.zeros(length)
    tmp_A = numpy.zeros(length)
    tmp_B = numpy.zeros(length)
    tmp_C = numpy.zeros(length)

    generator_ref = BLSawtooth(sampling_freq)
    generator_ref.SetFrequency(freq)
    generator_nopostfilter = BLSawtooth(sampling_freq, False)
    generator_nopostfilter.SetFrequency(freq)
    for idx in range(length):
        ref_data[idx] = generator_ref.ProcessSample()
        nopostfilter_data[idx] = generator_nopostfilter.ProcessSample()
        tmp_A[idx] = generator_ref.debug_A
        tmp_B[idx] = generator_ref.debug_B
        tmp_C[idx] = generator_ref.debug_C

    # filter = BLPostFilter()
    # test_data = numpy.zeros(length)
    # in_data = 2.0 * numpy.random.rand(length) - 1.0
    # for idx in range(length):
    #     value = in_data[idx]
    #     test_data[idx] = filter.process_sample(value)
    # pylab.plot(in_data, label="in")
    # pylab.plot(test_data)

    generator_left = BLSawtooth(sampling_freq)
    generator_left.SetFrequency(freq)
    for idx in range(length / 2):
        generated_data[idx] = generator_left.ProcessSample()

    generator_right = BLSawtooth(sampling_freq)
    generator_right.SetPhase(generated_data[length / 2 - 1])
    generator_right.SetFrequency(freq)
    generator_right.ProcessSample()
    for idx in range(length / 2, length):
        generated_data[idx] = generator_right.ProcessSample()

    print(utilities.PrintMetadata(utilities.GetMetadata(ref_data)))

    # pylab.plot(generator_ref._table, label="table")
    pylab.plot(ref_data, label="sawtooth")
    pylab.plot(nopostfilter_data, label="sawtooth_nopf")
    # pylab.plot(tmp_A, label="A")
    # pylab.plot(tmp_B, label="B")
    # pylab.plot(tmp_C, label="C")

    pylab.legend()
    pylab.show()

    utilities.WriteWav(ref_data / numpy.max(numpy.abs(ref_data)), "sawtooth_gen", sampling_freq)
    utilities.WriteWav(nopostfilter_data / numpy.max(numpy.abs(nopostfilter_data)), "sawtooth_gen_nopf", sampling_freq)
