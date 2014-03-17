# Retrieve all generators source files

file(GLOB
     SOUNDTAILOR_GENERATORS_SRC
     *.cc
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_GENERATORS_SRC
    ${SOUNDTAILOR_GENERATORS_SRC}
    PARENT_SCOPE
)

file(GLOB
     SOUNDTAILOR_GENERATORS_HDR
     *.h
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_GENERATORS_HDR
    ${SOUNDTAILOR_GENERATORS_HDR}
    PARENT_SCOPE
)
