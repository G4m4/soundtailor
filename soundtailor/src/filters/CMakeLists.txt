# Retrieve all filters source files

file(GLOB
     SOUNDTAILOR_FILTERS_SRC
     *.cc
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_FILTERS_SRC
    ${SOUNDTAILOR_FILTERS_SRC}
    PARENT_SCOPE
)

file(GLOB
     SOUNDTAILOR_FILTERS_HDR
     *.h
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_FILTERS_HDR
    ${SOUNDTAILOR_FILTERS_HDR}
    PARENT_SCOPE
)
