#!/usr/bin/env python
'''
@file extract_test_data.py
@brief Utilities for retrieving generated test data 
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

import numpy
import os

def ExtractSignal(filename):
    '''
    Open a file and extract a signal from it

    the file is supposed to be organized as follows:
    sample,\n
    sample,\n
    ...
    sample,\n

    Return a numpy signal
    '''
    if not os.path.exists(filename):
        raise Exception("No such file: "  + str(filename))
    return numpy.fromfile(filename, sep='\n')

if __name__ == "__main__":
    import pylab
    import utilities
    import os.path
    import scipy.io.wavfile

    files = ('../build/soundtailor/tests/in.dat',
             '../build/soundtailor/tests/out.dat',
#            '../build/soundtailor/tests/expected.dat',
             )

    view_beginning = 0
    view_length = 256

    signals = []
    for filename in files:
        signal_label = os.path.basename(filename)
        print ("File '" + signal_label + "': " + str(filename))
        signal = ExtractSignal(filename)
        metas = utilities.GetMetadata(signal)
        print(utilities.PrintMetadata(metas))
        pylab.plot(signal[view_beginning:view_beginning + view_length:1], label=signal_label)
        signals.append(signal)
        utilities.WriteWav(signal, filename, 96000.0)

    pylab.legend()

#     pylab.plot((utilities.Diff(signals[0]))[view_beginning:view_beginning + view_length:1])
    print(numpy.max(utilities.Diff(signals[0])))
    print(utilities.ZeroCrossings(utilities.Diff(signals[0])))
#     pylab.plot((signals[1] - signals[0])[view_beginning:view_beginning + view_length:1])
    pylab.show()
