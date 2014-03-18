# Retrieve all modulators source files

file(GLOB
     SOUNDTAILOR_MODULATORS_SRC
     *.cc
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_MODULATORS_SRC
    ${SOUNDTAILOR_MODULATORS_SRC}
    PARENT_SCOPE
)

file(GLOB
     SOUNDTAILOR_MODULATORS_HDR
     *.h
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_MODULATORS_HDR
    ${SOUNDTAILOR_MODULATORS_HDR}
    PARENT_SCOPE
)
