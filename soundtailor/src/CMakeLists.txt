# @brief SoundTailor main source folder cmake file

include_directories(
  ${SOUNDTAILOR_INCLUDE_DIR}
)

# Retrieve source files from subdirectories
add_subdirectory(filters)
add_subdirectory(generators)
add_subdirectory(modulators)
add_subdirectory(platform)

# Group sources
source_group("filters"
  FILES
  ${SOUNDTAILOR_FILTERS_SRC}
  ${SOUNDTAILOR_FILTERS_HDR}
)
source_group("generators"
  FILES
  ${SOUNDTAILOR_GENERATORS_SRC}
  ${SOUNDTAILOR_GENERATORS_HDR}
)
source_group("modulators"
  FILES
  ${SOUNDTAILOR_MODULATORS_SRC}
  ${SOUNDTAILOR_MODULATORS_HDR}
)
source_group("platform"
  FILES
  ${SOUNDTAILOR_PLATFORM_SRC}
  ${SOUNDTAILOR_PLATFORM_HDR}
)

# Sources
set(SOUNDTAILOR_SRC
  ${SOUNDTAILOR_FILTERS_SRC}
  ${SOUNDTAILOR_GENERATORS_SRC}
  ${SOUNDTAILOR_MODULATORS_SRC}
  ${SOUNDTAILOR_PLATFORM_SRC}
)
set(SOUNDTAILOR_HDR
  common.h
  configuration.h
  maths.h
  utilities.h
  ${SOUNDTAILOR_FILTERS_HDR}
  ${SOUNDTAILOR_GENERATORS_HDR}
  ${SOUNDTAILOR_MODULATORS_HDR}
  ${SOUNDTAILOR_PLATFORM_HDR}
)

# Target
add_library(soundtailor_lib
  ${SOUNDTAILOR_SRC}
  ${SOUNDTAILOR_HDR}
)

if (COMPILER_IS_GCC)
add_compiler_flags(soundtailor_lib "-Weffc++")
endif (COMPILER_IS_GCC)

set_target_mt(soundtailor_lib)
