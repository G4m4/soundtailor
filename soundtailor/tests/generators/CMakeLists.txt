# Retrieve all generators tests source files

file(GLOB
     SOUNDTAILOR_TESTS_GENERATORS_SRC
     *.cc
     *.h
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_TESTS_GENERATORS_SRC
    ${SOUNDTAILOR_TESTS_GENERATORS_SRC}
    PARENT_SCOPE
)

