# Retrieve all platform source files

file(GLOB
     SOUNDTAILOR_PLATFORM_SRC
     *.cc
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_PLATFORM_SRC
    ${SOUNDTAILOR_PLATFORM_SRC}
    PARENT_SCOPE
)

file(GLOB
     SOUNDTAILOR_PLATFORM_HDR
     *.h
)

# Expose variables to parent CMake files
set(SOUNDTAILOR_PLATFORM_HDR
    ${SOUNDTAILOR_PLATFORM_HDR}
    PARENT_SCOPE
)
