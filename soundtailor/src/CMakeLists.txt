# @brief SoundTailor main source folder cmake file

include_directories(
  ${SOUNDTAILOR_INCLUDE_DIR}
)

# Retrieve source files from subdirectories
add_subdirectory(filters)
add_subdirectory(generators)

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

# Sources
set(SOUNDTAILOR_SRC
  ${SOUNDTAILOR_FILTERS_SRC}
  ${SOUNDTAILOR_GENERATORS_SRC}
)
set(SOUNDTAILOR_HDR
  common.h
  configuration.h
  ${SOUNDTAILOR_FILTERS_HDR}
  ${SOUNDTAILOR_GENERATORS_HDR}
)

# Target
add_library(soundtailor_lib
  ${SOUNDTAILOR_SRC}
  ${SOUNDTAILOR_HDR}
)

set_target_mt(soundtailor_lib)
