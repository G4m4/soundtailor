# Retrieve all filters tests source files

file(GLOB
     SOUNDTAILOR_FILTERS_TESTS_SRC
     *.cc
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_FILTERS_TESTS_SRC
    ${SOUNDTAILOR_FILTERS_TESTS_SRC}
    PARENT_SCOPE
)

