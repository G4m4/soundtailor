# Retrieve all filters tests source files

file(GLOB
     SOUNDTAILOR_TESTS_FILTERS_SRC
     *.cc
     *.h
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_TESTS_FILTERS_SRC
    ${SOUNDTAILOR_TESTS_FILTERS_SRC}
    PARENT_SCOPE
)
