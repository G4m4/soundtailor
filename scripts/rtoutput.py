#!/usr/bin/env python
'''
@file rtoutput.py
@brief Real time audio output in order to test other modules
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

import numpy
import pyaudio
import time

import utilities
import generator_sin

def callback(in_data, frame_count, time_info, status):
    data = numpy.zeros(frame_count)
    for idx in range(frame_count):
        data[idx] = 0.5 * generator.ProcessSample()
    return (data.astype(numpy.float32), pyaudio.paContinue)

if __name__ == "__main__":
    '''
    Various tests/sandbox
    '''
    # instantiate PyAudio
    p = pyaudio.PyAudio()

    generator = generator_sin.SinGenerator(48000.0)
    generator.SetFrequency(440.0)
    # open stream using callback
    stream = p.open(format=p.get_format_from_width(4),
                    channels=1,
                    rate=48000,
                    output=True,
                    stream_callback=callback)

    # start the stream
    stream.start_stream()

    # wait for stream to finish
    while stream.is_active():
        time.sleep(0.1)

    # stop stream
    stream.stop_stream()
    stream.close()

    # close PyAudio
    p.terminate()
