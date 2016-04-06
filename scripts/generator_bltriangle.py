#!/usr/bin/env python
"""
@file generator_bltriangle.py
@brief Bandlimited triangle generation
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

from bandlimited_impulse import GenerateBLSawtoothIntegrate, BLPostFilter
from generators_common import GeneratorInterface, PhaseAccumulator, IncrementAndWrap


class BLTriangle(GeneratorInterface):
    """
    Implements a triangle signal generator based on BLIT algorithm
    """
    def __init__(self, sampling_rate, with_postfilter=True, ppiv=2700):
        super(BLTriangle, self).__init__(sampling_rate)
        self._table = GenerateBLSawtoothIntegrate(sampling_rate=sampling_rate, length = 4, ppiv=ppiv)
        self._halfM = len(self._table)
        self._sawtooth_gen = PhaseAccumulator(sampling_rate)
        self._alpha = 0.0
        self._frequency = 0.0
        self._phi = 0.0
        self._update = True
        if with_postfilter:
            self._post_filter = BLPostFilter()

    def SetPhase(self, phase):
        self._sawtooth_gen.SetPhase(phase)
        self._phi = phase
        if hasattr(self, '_post_filter'):
            self._post_filter.set_last(0.0)

    def SetFrequency(self, frequency):
        self._sawtooth_gen.SetFrequency(frequency)
        self._frequency = frequency

    def _read_table(self, value):
        abs_value = numpy.abs(value)
        if abs_value < self._alpha:
            relative_index = int(numpy.round(self._halfM * abs_value / self._alpha))
            index = numpy.minimum(self._halfM - relative_index, self._halfM - 1)
            if index > self._halfM / 2:
                read = self._table[self._halfM - index]
            else:
                read = self._table[index]
            # print(str(self) + str(value) + " " + str(relative_index) + " " + str(index) + "->" + str(read))
            return read
        else:
            return 0.0

    def ProcessSample(self):
        self._ProcessParameters()
        tmp = self._sawtooth_gen.ProcessSample()
        A = IncrementAndWrap(tmp, self._phi)
        B = IncrementAndWrap(A, 1.0)
        C = 2 * numpy.abs(A) - 1.0
        D = self._read_table(A)
        E = self._read_table(B)

        normalised_freq = 2 * self._frequency / self._sampling_rate
        F = C - D * normalised_freq + E * normalised_freq

        # out = numpy.clip(F, -1.0, 1.0)
        out = F

        if hasattr(self, '_post_filter'):
            return self._post_filter.process_sample(out)
        else:
            return out

    def _ProcessParameters(self):
        if self._update:
            self._alpha = self._sampling_rate * 4 / self._frequency
            self._update = False

if __name__ == "__main__":
    import numpy
    import pylab
    import utilities

    sampling_freq = 48000
    # Prime, as close as possible to the upper bound of 4kHz
    freq = 3989.0
    length = 240

    # Change phase
    generated_data = numpy.zeros(length)
    ref_data = numpy.zeros(length)
    nopostfilter_data = numpy.zeros(length)
    low_res_data = numpy.zeros(length)

    generator_ref = BLTriangle(sampling_freq)
    generator_ref.SetFrequency(freq)
    generator_nopostfilter = BLTriangle(sampling_freq, False)
    generator_nopostfilter.SetFrequency(freq)
    generator_low_res = BLTriangle(sampling_freq, ppiv=256)
    generator_low_res.SetFrequency(freq)
    for idx in range(length):
        ref_data[idx] = generator_ref.ProcessSample()
        nopostfilter_data[idx] = generator_nopostfilter.ProcessSample()
        low_res_data[idx] = generator_low_res.ProcessSample()

    generator_left = BLTriangle(sampling_freq)
    generator_left.SetFrequency(freq)
    for idx in range(length / 2):
        generated_data[idx] = generator_left.ProcessSample()

    generator_right = BLTriangle(sampling_freq)
    generator_right.SetPhase(generated_data[length / 2 - 1])
    generator_right.SetFrequency(freq)
    generator_right.ProcessSample()
    for idx in range(length / 2, length):
        generated_data[idx] = generator_right.ProcessSample()

    print(utilities.PrintMetadata(utilities.GetMetadata(ref_data)))
    # print(utilities.PrintMetadata(utilities.GetMetadata(ref_data - low_res_data)))

    # pylab.plot(generator_ref._table, label = "table")
    pylab.plot(ref_data, label = "triangle")
    # pylab.plot(nopostfilter_data, label = "triangle_nopf")
    pylab.plot(generated_data, label = "pieces_data")
    # pylab.plot(ref_data - low_res_data, label = "diff")

    pylab.legend()
    pylab.show()

    utilities.WriteWav(ref_data, "bl_triangle", sampling_freq)
    utilities.WriteWav(generated_data, "bl_triangle_phase", sampling_freq)
