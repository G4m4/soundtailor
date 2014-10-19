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

import filters_common
import filter_firstorderpolezero

class MoogBaseLowpass(filter_firstorderpolezero.FixedPoleZeroLowPass):
    '''
    Implements a simple 1 pole - 1 zero Low pass, with fixed coeffs tuned
    in order to be part of a bigger Moog filter
    '''
    def __init__(self):
        # The pole coeff will change, only the zero one is fixed here
        super(MoogBaseLowpass, self).__init__(0.3)

    def SetParameters(self, frequency, resonance):
        '''
        Sets frequency (normalized, < 1.0) and resonance (0 <= resonance < 4.0)
        '''
        self._pole_coeff = frequency

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        direct = self._pole_coeff / 1.3 * sample
        out = direct + self._last

        self._last = out * (1.0 - self._pole_coeff) + self._zero_coeff * direct

        return out

class Moog(filters_common.FilterInterface):
    '''
    Implements a Moog filter
    '''
    def __init__(self):
        self._frequency = 0.0
        self._resonance = 0.0
        self._last = 0.0
        self._filters = [MoogBaseLowpass() for i in xrange(4)]

    def SetParameters(self, frequency, resonance):
        '''
        Sets both frequency and resonance
        '''
        f = frequency * (1.0 + 0.03617 * frequency
                                        * (4.0 - resonance) * (4.0 - resonance))
        self._frequency = 1.25 * f * (1.0 - 0.595 * f + 0.24 * f * f)
        self._resonance = resonance * (1.0 + 0.077 * self._frequency
                                     - 0.117 * self._frequency * self._frequency
                                     - 0.049 * self._frequency * self._frequency * self._frequency)
        for lowpass in self._filters:
            lowpass.SetParameters(self._frequency, self._resonance)

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        actual_input = sample - self._resonance * self._last
        # Todo: find a more elegant way to do that
        tmp_filtered = actual_input
        for lowpass in self._filters:
            tmp_filtered = lowpass.ProcessSample(tmp_filtered)

        out = tmp_filtered
        self._last = out

        return out

class MoogMusicDSP(filters_common.FilterInterface):
    '''
    Implements a Moog filter based on MusicDSP source:
    http://musicdsp.org/showArchiveComment.php?ArchiveID=24
    '''
    def __init__(self):
        self.y1 = 0.0
        self.y2 = 0.0
        self.y3 = 0.0
        self.y4 = 0.0
        self.oldx = 0.0
        self.oldy1 = 0.0
        self.oldy2 = 0.0
        self.oldy3 = 0.0
        self.x = 0.0
        self.r = 0.0
        self.p = 0.0
        self.k = 0.0

    def SetParameters(self, frequency, resonance):
        '''
        Sets both frequency and resonance
        '''

        f = (frequency + frequency) #[0 - 1]
        self.p = f * (1.8 - 0.8 * f)
        # self.k = self.p + self.p - 1.0
        # A much better tuning seems to be:
        self.k = 2.0 * numpy.sin(f * numpy.pi * 0.5) - 1.0

        t = (1.0 - self.p) * 1.386249
        t2 = 12.0 + t * t
        self.r = resonance * (t2 + 6.0 * t) / (t2 - 6.0 * t)

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        # process input
        self.x = sample - self.r * self.y4

        # Four cascaded onepole filters (bilinear transform)
        self.y1 = self.x * self.p +  self.oldx * self.p - self.k * self.y1
        self.y2 = self.y1 * self.p + self.oldy1 * self.p - self.k * self.y2
        self.y3 = self.y2 * self.p + self.oldy2 * self.p - self.k * self.y3
        self.y4 = self.y3 * self.p + self.oldy3 * self.p - self.k * self.y4

        # Clipper band limited sigmoid
        self.y4 -= (self.y4 * self.y4 * self.y4) / 6.0

        self.oldx = self.x
        self.oldy1 = self.y1
        self.oldy2 = self.y2
        self.oldy3 = self.y3

        return self.y4

class MoogMusicDSPVar1(filters_common.FilterInterface):
    '''
    Implements a Moog filter based on MusicDSP source:
    http://musicdsp.org/showArchiveComment.php?ArchiveID=25
    '''
    def __init__(self):
        self.b0 = 0.0
        self.b1 = 0.0
        self.b2 = 0.0
        self.b3 = 0.0
        self.b4 = 0.0
        self.f = 0.0
        self.p = 0.0
        self.q = 0.0

    def SetParameters(self, frequency, resonance):
        '''
        Sets both frequency and resonance
        '''
        self.q = 1.0 - frequency
        self.p = frequency + 0.8 * frequency * self.q
        self.f = self.p + self.p - 1.0
        self.q = resonance * (1.0 + 0.5 * self.q * (1.0 - self.q + 5.6 * self.q * self.q))

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        # Feedback
        sample -= self.q * self.b4

        t1 = self.b1
        self.b1 = (sample + self.b0) * self.p - self.b1 * self.f
        t2 = self.b2
        self.b2 = (self.b1 + t1) * self.p - self.b2 * self.f
        t1 = self.b3
        self.b3 = (self.b2 + t2) * self.p - self.b3 * self.f

        self.b4 = (self.b3 + t1) * self.p - self.b4 * self.f
        # Clipping
        self.b4 = self.b4 - self.b4 * self.b4 * self.b4 * 0.166667
        self.b0 = sample

        return self.b4

class MoogMusicDSPVar2(filters_common.FilterInterface):
    '''
    Implements a Moog filter based on MusicDSP source:
    http://musicdsp.org/showArchiveComment.php?ArchiveID=26
    '''
    def __init__(self):
        self.out1 = 0.0
        self.out2 = 0.0
        self.out3 = 0.0
        self.out4 = 0.0
        self.in1 = 0.0
        self.in2 = 0.0
        self.in3 = 0.0
        self.in4 = 0.0
        self.f = 0.0
        self.fb = 0.0

    def SetParameters(self, frequency, resonance):
        '''
        Sets both frequency and resonance
        '''
        self.f = frequency * 1.16
        self.fb = resonance * (1.0 - 0.15 * self.f * self.f)

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        # Feedback
        sample -= self.out4 * self.fb
        sample *= 0.35013 * (self.f * self.f) * (self.f * self.f)
        self.out1 = sample + 0.3 * self.in1 + (1 - self.f) * self.out1
        self.in1 = sample
        self.out2 = self.out1 + 0.3 * self.in2 + (1 - self.f) * self.out2
        self.in2 = self.out1
        self.out3 = self.out2 + 0.3 * self.in3 + (1 - self.f) * self.out3
        self.in3 = self.out2
        self.out4 = self.out3 + 0.3 * self.in4 + (1 - self.f) * self.out4
        self.in4 = self.out3

        return self.out4

class MoogMusicDSPVarStilson(filters_common.FilterInterface):
    '''
    Implements a Moog filter based on MusicDSP source:
    http://musicdsp.org/showArchiveComment.php?ArchiveID=145
    '''
    GAINTABLE = [0.999969, 0.990082, 0.980347, 0.970764, 0.961304, 0.951996, 0.94281, 0.933777, 0.924866, 0.916077, 0.90741, 0.898865,
                 0.890442, 0.882141 , 0.873962, 0.865906, 0.857941, 0.850067, 0.842346, 0.834686, 0.827148, 0.819733, 0.812378, 0.805145,
                 0.798004, 0.790955, 0.783997, 0.77713, 0.770355, 0.763672, 0.75708 , 0.75058, 0.744141, 0.737793, 0.731537, 0.725342,
                 0.719238, 0.713196, 0.707245, 0.701355, 0.695557, 0.689819, 0.684174, 0.678558, 0.673035, 0.667572, 0.66217, 0.65686,
                 0.651581, 0.646393, 0.641235, 0.636169, 0.631134, 0.62619, 0.621277, 0.616425, 0.611633, 0.606903, 0.602234, 0.597626,
                 0.593048, 0.588531, 0.584045, 0.579651, 0.575287 , 0.570953, 0.566681, 0.562469, 0.558289, 0.554169, 0.550079, 0.546051,
                 0.542053, 0.538116, 0.53421, 0.530334, 0.52652, 0.522736, 0.518982, 0.515289, 0.511627, 0.507996 , 0.504425, 0.500885,
                 0.497375, 0.493896, 0.490448, 0.487061, 0.483704, 0.480377, 0.477081, 0.473816, 0.470581, 0.467377, 0.464203, 0.46109,
                 0.457977, 0.454926, 0.451874, 0.448883, 0.445892, 0.442932, 0.440033, 0.437134, 0.434265, 0.431427, 0.428619, 0.425842,
                 0.423096, 0.42038, 0.417664, 0.415009, 0.412354, 0.409729, 0.407135, 0.404572, 0.402008, 0.399506, 0.397003, 0.394501,
                 0.392059, 0.389618, 0.387207, 0.384827, 0.382477, 0.380127, 0.377808, 0.375488, 0.37323, 0.370972, 0.368713, 0.366516,
                 0.364319, 0.362122, 0.359985, 0.357849, 0.355713, 0.353607, 0.351532, 0.349457, 0.347412, 0.345398, 0.343384, 0.34137,
                 0.339417, 0.337463, 0.33551, 0.333588, 0.331665, 0.329773, 0.327911, 0.32605, 0.324188, 0.322357, 0.320557, 0.318756,
                 0.316986, 0.315216, 0.313446, 0.311707, 0.309998, 0.308289, 0.30658, 0.304901, 0.303223, 0.301575, 0.299927, 0.298309,
                 0.296692, 0.295074, 0.293488, 0.291931, 0.290375, 0.288818, 0.287262, 0.285736, 0.284241, 0.282715, 0.28125, 0.279755,
                 0.27829, 0.276825, 0.275391, 0.273956, 0.272552, 0.271118, 0.269745, 0.268341, 0.266968, 0.265594, 0.264252, 0.262909,
                 0.261566, 0.260223, 0.258911, 0.257599, 0.256317, 0.255035, 0.25375]

    def __init__(self):
#         self.out1 = 0.0
#         self.out2 = 0.0
#         self.out3 = 0.0
#         self.out4 = 0.0
#         self.in1 = 0.0
#         self.in2 = 0.0
#         self.in3 = 0.0
        self.last_out = 0.0
        self.state = [0.0, 0.0, 0.0, 0.0]
        self.Q = 0.0
        self.p = 0.0

    def _saturate(self, sample):
        '''
        clamp without branching
        '''
        _limit = 0.95
        x1 = numpy.abs(sample + _limit)
        x2 = numpy.abs(sample - _limit)

        return 0.5 * (x1 - x2)

    def _crossfade(self, amount, a, b):
        return (1.0 - amount) * a + amount * b

    def SetParameters(self, frequency, resonance):
        '''
        Sets both frequency and resonance
        '''

        # code for setting Q
        ix = self.p * 99.0
        ixint = numpy.floor(ix).astype(numpy.int32)
        ixfrac = ix - ixint
        # The following are identical
#         self.Q = resonance * self._crossfade( ixfrac, self.GAINTABLE[ixint + 99], self.GAINTABLE[ixint + 100])
        self.Q = resonance * 1 / (ixfrac * 1.48 + 0.85) - 0.1765

        # code for setting pole coefficient based on frequency
        fc = 2 * frequency
        x2 = fc * fc
        x3 = fc * x2
        # cubic fit by DFL, not 100% accurate but better than nothing...
        self.p = -0.69346 * x3 - 0.59515 * x2 + 3.2937 * fc - 1.0072

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        # negative feedback
        self.last_out = 0.25 * (sample - self.last_out)

        for pole in range(4):
            temp = self.state[pole]
            self.output = self._saturate(self.output + self.p * (self.output - temp))
            self.state[pole] = self.output
            self.output = self._saturate(self.output + temp)

        lowpass = self.output
        highpass = sample - self.output
        # got this one from paul kellet
        bandpass = 3.0 * self.state[2] - lowpass
        out = lowpass

        # scale the feedback
        self.output *= self.Q

        return out

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    import numpy
    import pylab
    import utilities
    from scipy.io.wavfile import read

    import generator_sawtoothdpw

    freq = 89.0
    sampling_freq = 48000.0
    length = 4096
    filter_freq = 1000.0 / sampling_freq
    resonance = 0.9

    in_data = utilities.GenerateData(100, 2000, length, sampling_freq)
    generator = generator_sawtoothdpw.SawtoothDPW(sampling_freq)
    generator.SetFrequency(freq)
    for idx, _ in enumerate(in_data):
        in_data[idx] = generator.ProcessSample()
    (_, in_data) = read("f2_20khz_emphasis_0_no_soft_clipping.wav")
    in_data = numpy.array(list(in_data),dtype='float') / numpy.max(in_data)
    in_data = in_data[0:length]

    (_, ref_data) = read("f2_1khz_emphasis_0.wav")
    ref_data = numpy.array(list(ref_data),dtype='float') / numpy.max(ref_data)
    ref_data = ref_data[0:length]

    #in_data = numpy.random.rand(length) * 2.0 - 1.0
    out_data = numpy.zeros(length)
    out_base_lowpass = numpy.zeros(length)
    out_data_ref1 = numpy.zeros(length)
    out_data_ref2 = numpy.zeros(length)
    out_data_ref3 = numpy.zeros(length)
    out_data_ref4 = numpy.zeros(length)

    lowpass = Moog()
    lowpass.SetParameters(filter_freq, resonance)
    # The base lowpass has to be updated with the actual internal values
    base_lowpass = MoogBaseLowpass()
    base_lowpass.SetParameters(lowpass._frequency, lowpass._resonance)
    # Comparison
    ref1_lowpass = MoogMusicDSP()
    ref1_lowpass.SetParameters(lowpass._frequency, lowpass._resonance)
    ref2_lowpass = MoogMusicDSPVar1()
    ref2_lowpass.SetParameters(lowpass._frequency, lowpass._resonance)
    ref3_lowpass = MoogMusicDSPVar2()
    ref3_lowpass.SetParameters(lowpass._frequency, lowpass._resonance)
    ref4_lowpass = MoogMusicDSPVarStilson()
    ref4_lowpass.SetParameters(lowpass._frequency, lowpass._resonance)

    for idx, _ in enumerate(in_data):
        out_data[idx] = lowpass.ProcessSample(in_data[idx])
        out_base_lowpass[idx] = base_lowpass.ProcessSample(in_data[idx])
        out_data_ref1[idx] = ref1_lowpass.ProcessSample(in_data[idx])
        out_data_ref2[idx] = ref2_lowpass.ProcessSample(in_data[idx])
        out_data_ref3[idx] = ref3_lowpass.ProcessSample(in_data[idx])
        out_data_ref4[idx] = ref3_lowpass.ProcessSample(in_data[idx])

    pylab.plot(in_data, label="in")
#     pylab.plot(out_base_lowpass, label="out_baselp")
    pylab.plot(out_data, label="out")
    pylab.plot(out_data_ref1, label="out_ref1")
    pylab.plot(out_data_ref2, label="out_ref2")
    pylab.plot(out_data_ref3, label="out_ref3")
    pylab.plot(out_data_ref4, label="out_ref4")
    pylab.legend()
    pylab.show()
