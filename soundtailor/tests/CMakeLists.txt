# @brief Build SoundTailor tests executable

# preventing warnings from external source files
include_directories(
  SYSTEM
  ${GTEST_INCLUDE_DIRS}
)

include_directories(
  ${SOUNDTAILOR_INCLUDE_DIR}
)

# Include all subdirectories tests source files
add_subdirectory(generators)

# Group sources
source_group("generators"
  FILES
  ${SOUNDTAILOR_TESTS_GENERATORS_SRC}
)

# Source files
set(SOUNDTAILOR_TESTS_SRC
    main.cc
    ${SOUNDTAILOR_TESTS_GENERATORS_SRC}
)

# Target
add_executable(soundtailor_tests
  ${SOUNDTAILOR_TESTS_SRC}
)

set_target_mt(soundtailor_tests)

target_link_libraries(soundtailor_tests
  soundtailor_lib
  gtest_main
)
