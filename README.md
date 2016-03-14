SoundTailor - Sound synthesis algorithms
==================================

[![Build Status](https://travis-ci.org/G4m4/soundtailor.svg?branch=master)](https://travis-ci.org/G4m4/soundtailor)

SoundTailor is a library dedicated to sound synthesis algorithms.
It aims at showing a simple, cross-platform yet efficient implementation of state of the art audio synthesis algorithms.

As its main goal is education, information sharing and (hopefully) inspiring other people to hack and spread audio synthesis tools and art, it is licensed under GPLv3 license.
It basically means that every one can use, copy and modify it with the sole condition that the source code must be made public.

Organization
------------

Most of the algorithms are those described in the 'Digital Sound Generation' online book, published by the Institute for Computer Music and Sound Technology.

The implementation itself is first prototyped in Python - scripts are made public under the same license.

The general philosophy is simplicity: most of what is proposed here is quite dumb by itself, it needs to be embedded into an actual project in order to output sound.
A quite exhaustive demonstration is being developed in the [OpenMini project](http://g4m4.github.io/openmini/) as a complete virtual analog synthesizer.

All of this code is heavily tested using the [Google Test Framework](http://code.google.com/p/googletest/).
It is documented following Doxygen convention, and strictly follows Google style - cpplint script is included for this purpose.
All the code is standard C++: no platform-specific headers.

Note that SoundTailor is under continuous integration, building under Linux (using gcc and Clang) at each push with [Travis CI](https://travis-ci.org/G4m4/soundtailor).
The Windows build is continuously tested as well.

Building SoundTailor library
-------------------------

SoundTailor build system is Cmake.

Its most basic use would be, from the root directory soundtailor/ :

    mkdir build && cd build
    cmake ..
    cmake --build .

This only build the library - nothing else. You can also build SoundTailor tests as explained below.

Building SoundTailor tests
-----------------------

Tests are using the [Google Test Framework](http://code.google.com/p/googletest/).
To retrieve the library files in order to build tests, the following must be done from the root folder:

    git submodule init
    git submodule update

Once this is done, you only have to set the flag SOUNDTAILOR_HAS_GTEST to ON (caps matters) when invoking cmake:

    cmake -DSOUNDTAILOR_HAS_GTEST=ON ../

License
==================================
SoundTailor is under GPLv3.

See the COPYING file for the full license text.
