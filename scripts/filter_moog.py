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
        return super(MoogBaseLowpass, self).ProcessSample(sample * 2.0 / 1.3)

    def Process4Samples(self, vector):
        return super(MoogBaseLowpass, self).Process4Samples(filters_common.MulConst(vector, 2.0 / 1.3))


class MoogLowAliasNonLinearBaseLowpass(filter_firstorderpolezero.FixedPoleZeroLowPass):
    '''
    Implements a simple 1 pole - 1 zero Low pass, with fixed coeffs tuned
    in order to be part of a bigger Moog filter
    Same as FixedPoleZeroLowPass but with a different input factor (* 2.0)
    '''
    def __init__(self):
        # The pole coeff will change, only the zero one is fixed here
        super(MoogLowAliasNonLinearBaseLowpass, self).__init__(0.3)

    def SetParameters(self, frequency, resonance):
        '''
        Sets frequency (normalized, < 1.0) and resonance (0 <= resonance < 4.0)
        '''
        self._pole_coeff = frequency

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        return super(MoogLowAliasNonLinearBaseLowpass, self).ProcessSample(sample * 2.0)

    def Process4Samples(self, vector):
        '''
        Actual process function
        '''
        return super(MoogLowAliasNonLinearBaseLowpass, self).Process4Samples(filters_common.MulConst(vector, 2.0))


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
        self._last = tmp_filtered

        return out

    def Process4Samples(self, vector):
        """
        Actual process function
        """
        out = [0.0, 0.0, 0.0, 0.0]
        last = self._last
        for idx, sample in enumerate(vector):
            actual_input = sample - self._resonance * last
            # Todo: find a more elegant way to do that
            tmp_filtered = actual_input
            for lowpass in self._filters:
                tmp_filtered = lowpass.ProcessSample(tmp_filtered)

            out[idx] = tmp_filtered
            last = tmp_filtered
        self._last = last

        return out

class MoogLowAliasNonLinear(filters_common.FilterInterface):
    '''
    Implements a low alias, non-linear Moog filter
    '''
    def __init__(self):
        self._frequency = 0.0
        self._resonance = 0.0
        self._last = 0.0
        self._filters = [MoogLowAliasNonLinearBaseLowpass() for i in xrange(4)]
        self._last_side_factor = 0.0

    def _Saturation(self, sample):
        if (numpy.abs(sample) < 1.0):
            return sample - sample * sample * sample / 3.0
        else:
            return (2.0 / 3.0) * min(max(sample, -1.0), 1.0)

    def SetParameters(self, frequency, resonance):
        '''
        Sets both frequency and resonance
        '''
        # This filter corrected resonance should be within [0.0 ; 1.0]
        # For consistency with other Moog filters it is scaled down here
        resonance /= 4.0
        f = frequency * (1.0 + 0.5787 * frequency
                                        * (1.0 - resonance) * (1.0 - resonance))
        self._frequency = 1.25 * f * (1.0 - 0.595 * f + 0.24 * f * f)
        self._resonance = resonance * (1.4 + 0.108 * self._frequency
                                     - 0.164 * self._frequency * self._frequency
                                     - 0.069 * self._frequency * self._frequency * self._frequency)
        for lowpass in self._filters:
            lowpass.SetParameters(self._frequency, self._resonance)

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        sample *= 0.18 + 0.25 * self._resonance
        actual_input = sample - self._resonance * self._last
        current_side_factor = min(max(self._last_side_factor, -1.0), 1.0)

        self._last_side_factor = actual_input * actual_input
        self._last_side_factor *= 0.062
        self._last_side_factor += current_side_factor * 0.993

        current_side_factor = 1.0 - current_side_factor + current_side_factor * current_side_factor / 2.0
        actual_input *= current_side_factor

        # Todo: find a more elegant way to do that
        tmp_filtered = actual_input
        for lowpass in self._filters[0:2]:
            tmp_filtered = lowpass.ProcessSample(tmp_filtered)

        # Saturation
        tmp_filtered = self._Saturation(tmp_filtered)
        for lowpass in self._filters[2:4]:
            tmp_filtered = lowpass.ProcessSample(tmp_filtered)

        out = tmp_filtered
        self._last = tmp_filtered

        return out

    def Process4Samples(self, vector):
        """
        Actual process function
        """
        out = [0.0, 0.0, 0.0, 0.0]
        direct_v = filters_common.MulConst(vector, 0.18 + 0.25 * self._resonance)
        last = self._last
        for idx, sample in enumerate(direct_v):
            actual_input = sample - self._resonance * last
            current_side_factor = min(max(self._last_side_factor, -1.0), 1.0)

            self._last_side_factor = actual_input * actual_input
            self._last_side_factor *= 0.062
            self._last_side_factor += current_side_factor * 0.993

            current_side_factor = 1.0 - current_side_factor + current_side_factor * current_side_factor / 2.0
            actual_input *= current_side_factor

            # Todo: find a more elegant way to do that
            tmp_filtered = actual_input
            for lowpass in self._filters[0:2]:
                tmp_filtered = lowpass.ProcessSample(tmp_filtered)

            # Saturation
            tmp_filtered = self._Saturation(tmp_filtered)
            for lowpass in self._filters[2:4]:
                tmp_filtered = lowpass.ProcessSample(tmp_filtered)

            out[idx] = tmp_filtered
            last = tmp_filtered
        self._last = last

        return out


class MoogLowAliasNonLinearOversampled(MoogLowAliasNonLinear):
    '''
    Implements a naive oversampling of the low alias, non-linear Moog filter
    '''
    def __init__(self):
        super(MoogLowAliasNonLinearOversampled, self).__init__()

    def ProcessSample(self, sample):
        '''
        2x oversampled process function
        '''
        a = super(MoogLowAliasNonLinearOversampled, self).ProcessSample(sample)
        b = super(MoogLowAliasNonLinearOversampled, self).ProcessSample(sample)
        return b

    def Process4Samples(self, vector):
        '''
        2x oversampled process function
        '''
        new_vec_first = numpy.array((vector[0], vector[0], vector[1], vector[1]))
        new_vec_second = numpy.array((vector[2], vector[2], vector[3], vector[3]))
        first_half = super(MoogLowAliasNonLinearOversampled, self).Process4Samples(new_vec_first)
        second_half = super(MoogLowAliasNonLinearOversampled, self).Process4Samples(new_vec_second)
        return filters_common.Decimate(first_half, second_half)


class MoogOversampled(MoogLowAliasNonLinear):
    '''
    Implements an oversampled low alias, non-linear Moog filter
    '''
    def __init__(self):
        super(MoogOversampled, self).__init__()
        self._filter_output = numpy.array([0.0, 0.0, 0.0, 0.0])
        self._last_out = 0.0

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        history_coeffs = numpy.array([0.19, 0.57, 0.57, 0.19])
        out = super(MoogOversampled, self).ProcessSample(sample)
        # The following are useless if we use the same input twice
        # self._filter_output[3] = self._filter_output[2]
        # self._filter_output[2] = self._filter_output[1]
        # self._filter_output[1] = self._filter_output[0]
        # self._filter_output[0] = out
        # 2x oversampled
        out = super(MoogOversampled, self).ProcessSample(sample)
        history = self._filter_output
        new_history = filters_common.RotateOnRight(history, out)
        # self._filter_output[3] = self._filter_output[2]
        # self._filter_output[2] = self._filter_output[1]
        # self._filter_output[1] = self._filter_output[0]
        # self._filter_output[0] = out
        out = numpy.sum(history_coeffs * new_history)

        out += self._last_out * -0.52
        self._filter_output = new_history
        self._last_out = out

        return out

    def Process4Samples(self, vector):
        out = numpy.zeros(len(vector))
        out[0] = self.ProcessSample(vector[0])
        out[1] = self.ProcessSample(vector[1])
        out[2] = self.ProcessSample(vector[2])
        out[3] = self.ProcessSample(vector[3])
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

#         f = (frequency + frequency) #[0 - 1]
        f = frequency
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
        frequency /= 2.0
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
        self.output = 0.0
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

        # code for setting pole coefficient based on frequency
#         fc = 2.0 * frequency
        fc = frequency
        x2 = fc * fc
        x3 = fc * x2
        # cubic fit by DFL, not 100% accurate but better than nothing...
        self.p = -0.69346 * x3 - 0.59515 * x2 + 3.2937 * fc - 1.0072

        # code for setting Q
        ix = self.p * 99.0
        ixint = numpy.floor(ix).astype(numpy.int32)
        ixfrac = ix - ixint
        # The following are identical
        self.Q = resonance * self._crossfade( ixfrac, self.GAINTABLE[ixint + 99], self.GAINTABLE[ixint + 100])
#         self.Q = resonance * 1.0 / (ixfrac * 1.48 + 0.85) - 0.1765

    def ProcessSample(self, sample):
        '''
        Actual process function
        '''
        # negative feedback
        self.output = 0.25 * (sample - self.output)

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
    sampling_freq = 96000.0
    length = 1024
    resonance = 0.0
    filter_freq = sampling_freq / sampling_freq

    in_data = utilities.GenerateChirpData(100, 2000, length, sampling_freq)
#     generator = generator_sawtoothdpw.SawtoothDPW(sampling_freq)
#     generator.SetFrequency(freq)
#     for idx, _ in enumerate(in_data):
#         in_data[idx] = generator.ProcessSample()
    (_, in_data) = read("f2_20khz_emphasis_0_no_soft_clipping.wav")
    in_data = numpy.array(list(in_data),dtype='float') / numpy.max(in_data)
    in_data = in_data[0:length]
    in_data = 2.0 * numpy.random.rand(length) - 1.0
    in_data = utilities.GenerateSquareData(1000, length, sampling_freq)
#     in_data = numpy.ones(length)

    (_, ref_data) = read("f2_1khz_emphasis_10_no_soft_clipping.wav")
    ref_data = numpy.array(list(ref_data),dtype='float') / numpy.max(ref_data)
    ref_data = ref_data[0:length]

    available_filters = [
                          # MoogLowAliasNonLinearBaseLowpass,
                          # MoogBaseLowpass,
                         # Moog,
                         MoogLowAliasNonLinear,
                         MoogLowAliasNonLinearOversampled,
                         MoogOversampled,
#                          MoogMusicDSP,
#                          MoogMusicDSPVar1,
#                          MoogMusicDSPVar2,
#                          MoogMusicDSPVarStilson
                         ]

#     in_data = numpy.random.rand(length) * 2.0 - 1.0
    out_data = numpy.zeros([len(available_filters), length])

    for filter_idx, filter_class in enumerate(available_filters):
        filter_instance = filter_class()
        filter_instance_v = filter_class()
        filter_instance.SetParameters(filter_freq, resonance)
        filter_instance_v.SetParameters(filter_freq, resonance)
        # The base lowpass has to be updated with the actual internal values
        for idx, _ in enumerate(in_data):
            out_data[filter_idx][idx] = filter_instance.ProcessSample(in_data[idx])
        # Check vectorized version
        squared_diff = 0.0
        idx = 0
        while idx < len(in_data) - 4:
            current_vector = (in_data[idx],
                              in_data[idx + 1],
                              in_data[idx + 2],
                              in_data[idx + 3])
            current_out = numpy.array(filter_instance_v.Process4Samples(current_vector))
            cmp = numpy.array(out_data[filter_idx][idx:idx+4])
            squared_diff += numpy.sum(current_out * current_out - cmp * cmp)
            idx += 4
        filter_name = str(type(filter_instance))
        pylab.plot(out_data[filter_idx], label="out" + filter_name)
        print(filter_name + utilities.PrintMetadata(utilities.GetMetadata(out_data[filter_idx])))
        print("Squared diff: " + str(squared_diff))

    print("in_data: " + utilities.PrintMetadata(utilities.GetMetadata(in_data)))
    diff = in_data - out_data[filter_idx]
    pylab.plot(diff)
    print("diff: " + utilities.PrintMetadata(utilities.GetMetadata(diff)))
    pylab.plot(in_data, label="in")

    pylab.legend()
    pylab.show()
