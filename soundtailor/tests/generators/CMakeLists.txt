# Retrieve all generators tests source files

file(GLOB
     SOUNDTAILOR_GENERATORS_TESTS_SRC
     *.cc
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_GENERATORS_TESTS_SRC
    ${SOUNDTAILOR_GENERATORS_TESTS_SRC}
    PARENT_SCOPE
)

