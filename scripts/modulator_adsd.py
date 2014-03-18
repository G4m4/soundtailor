#!/usr/bin/env python
'''
@file modulator_envelopgenerator.py
@brief Simple envelop generator - ADSD model
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
import utilities

'''
Note that all of this mimics C++ code for testing/prototyping purpose.
Hence it may not really seems "pythonic" and not intended to be in any way.
'''

from modulators_common import EnvelopGeneratorInterface

class ADSD(EnvelopGeneratorInterface):
    '''
    Implements a attack-decay-sustain-decay envelop generator, mimicking
    the one of the Minimoog
    '''
    kMaxAmplitude = 1.0
    def __init__(self, sampling_rate):
        super(ADSD, self).__init__(sampling_rate)
        self._cursor = 0
        self._current_increment = 0.0
        self._current_value = 0.0
        self._sustain_level = 0.0
        self._section = 0
        self._attack = 0
        self._decay = 0
        self._actual_decay = 0
        self._actual_release = 0

    def SetParameters(self, attack, decay, sustain_level, release):
        self._attack = attack
        self._decay = decay
        self._actual_decay = decay + attack
        self._sustain_level = sustain_level
        self._section = 4

    def TriggerOn(self):
        self._cursor = 0
        self._section = 0
        rise = self.kMaxAmplitude
        self._current_increment = self._ComputeIncrement(rise, attack)

    def TriggerOff(self):
        self._section = 3
        rise = -self._current_value
        run = self._decay
        self._current_increment = self._ComputeIncrement(rise, run)
        self._actual_release = self._cursor + self._decay

    def ProcessSample(self):
        out = self._current_value
        self._cursor += 1
        if self._section == 0:
            if (self._cursor > self._attack):
                self._section += 1
                rise = self._sustain_level - self.kMaxAmplitude
                run = self._decay
                self._current_increment = self._ComputeIncrement(rise, run)
                self._current_value = self.kMaxAmplitude + self._current_increment
            else:
                self._current_value += self._current_increment
        elif self._section == 1:
            if (self._cursor > self._actual_decay):
                self._section += 1
                self._current_value = self._sustain_level
            else:
                self._current_value += self._current_increment
        elif self._section == 2:
            # Sustain case
            pass
        elif self._section == 3:
            if (self._cursor > self._actual_release):
                self._section += 1
            else:
                self._current_value += self._current_increment
        elif self._section == 4:
            # Out of the envelop case
            return 0.0
        else:
            raise Exception("This should not happen!")

        return out

    def _ComputeIncrement(self, rise, run):
        '''
        Helper function for computing the increment at each increment,
        given the rise (vertical change) and run (horizontal change)
        '''
        if 0 == run:
            return rise
        return rise / run

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import numpy
    import pylab
    import generators_common

    sampling_freq = 48000

    attack = 0
    decay = 0
    sustain = 27939
    sustain_level = 0.547215521
    release = 125
    tail = 256

    TriggerOccurence = attack + decay * 2 + sustain + tail
    TriggerLength = attack + decay + sustain

    view_beginning = 0
    view_length = 30000

    generator = ADSD(sampling_freq)
    generator.SetParameters(attack, decay, sustain_level, release)
    generated_data = numpy.zeros(view_length)
    was_triggered = False
    for idx, _ in enumerate(generated_data):
        if was_triggered and (idx % TriggerLength) == 0:
            generator.TriggerOff()
            was_triggered = False
        if (idx % TriggerOccurence) == 0:
            generator.TriggerOn()
            was_triggered = True
        generated_data[idx] = generator.ProcessSample()

    pylab.plot(generated_data[view_beginning:view_length])

    differentiator = generators_common.Differentiator()
    diff_data = numpy.zeros(len(generated_data))
    for idx, sample in enumerate(generated_data):
        diff_data[idx] = differentiator.ProcessSample(sample)

    print(utilities.ZeroCrossings(diff_data))

    pylab.plot(diff_data)
    pylab.show()
