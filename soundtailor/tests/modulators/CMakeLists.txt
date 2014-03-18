# Retrieve all modulators tests source files

file(GLOB
     SOUNDTAILOR_TESTS_MODULATORS_SRC
     *.cc
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_TESTS_MODULATORS_SRC
    ${SOUNDTAILOR_TESTS_MODULATORS_SRC}
    PARENT_SCOPE
)

