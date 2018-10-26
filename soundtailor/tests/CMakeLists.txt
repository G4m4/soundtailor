# @brief Build SoundTailor tests executable

# preventing warnings from external source files
include_directories(
  SYSTEM
  ${GTEST_INCLUDE_DIRS}
  ${VECMATH_INCLUDE_DIRS}
)

include_directories(
  ${SOUNDTAILOR_INCLUDE_DIR}
)

# Include all subdirectories tests source files
add_subdirectory(filters)
add_subdirectory(generators)
add_subdirectory(modulators)

# Group sources
source_group("filters"
  FILES
  ${SOUNDTAILOR_TESTS_FILTERS_SRC}
)
source_group("generators"
  FILES
  ${SOUNDTAILOR_TESTS_GENERATORS_SRC}
)
source_group("modulators"
  FILES
  ${SOUNDTAILOR_TESTS_MODULATORS_SRC}
)

# Source files
set(SOUNDTAILOR_TESTS_SRC
    main.cc
    ${SOUNDTAILOR_TESTS_FILTERS_SRC}
    ${SOUNDTAILOR_TESTS_GENERATORS_SRC}
    ${SOUNDTAILOR_TESTS_MODULATORS_SRC}
)
set(SOUNDTAILOR_TESTS_HDR
    analysis.h
    debug.h
    tests.h
)

# Target
add_executable(soundtailor_tests
  ${SOUNDTAILOR_TESTS_SRC}
  ${SOUNDTAILOR_TESTS_HDR}
)

set_target_mt(soundtailor_tests)

target_link_libraries(soundtailor_tests
  soundtailor_lib
  gtest_main
)
