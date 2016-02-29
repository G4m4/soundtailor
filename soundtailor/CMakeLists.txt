# @brief SoundTailor root cmake file

add_subdirectory(src)

if (SOUNDTAILOR_HAS_GTEST)
  add_subdirectory(tests)
endif (SOUNDTAILOR_HAS_GTEST)
