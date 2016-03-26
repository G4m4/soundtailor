/// @filename debug.hc
/// @brief Tests common debug include file
/// @author gm
/// @copyright gm 2016
///
/// This file is part of SoundTailor
///
/// SoundTailor is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// SoundTailor is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with SoundTailor.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SOUNDTAILOR_TESTS_DEBUG_H_
#define SOUNDTAILOR_TESTS_DEBUG_H_

// offsetof
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "soundtailor/src/configuration.h"
#include "soundtailor/src/maths.h"

#if _COMPILER_MSVC
#pragma warning(push)
// Equivalent to _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif  // _COMPILER_MSVC

/// @brief Namespace holding debug stuff - nothing using it should be committed
namespace debug {

/// @brief Helper class for writing signal data into files
///
/// This is clearly not optimized at all nor really clean,
/// for debugging purpose only!
class FileWriter {
 public:
  explicit FileWriter(const char* filename)
    : out_(std::fopen(filename, "w+")) {
    // Nothing to do here for now
  }
  ~FileWriter() {
    Close();
  }

  /// @brief Push one sample into file writer
  void Push(const Sample sample) {
    // Using the slower runtime version of GetByIndex() for genericity's sake
    for (unsigned int index(0); index < soundtailor::SampleSize; index += 1) {
      PushFloat(VectorMath::GetByIndex(sample, index));
    }
  }

  /// @brief Push version for one actual value
  void PushFloat(const float sample) {
    std::fprintf(out_, "%1.7f\n", sample);
  }

  /// @brief Push version for an entire buffer
  void PushBuffer(const float* buffer, const unsigned int buffer_size) {
    for (unsigned int i(0); i < buffer_size; ++i) {
      PushFloat(buffer[i]);
    }
  }

  /// @brief Close writer (automatically done when destroyed)
  void Close(void) {
    if (out_) {
      std::fclose(out_);
      out_ = nullptr;
    }
  }

 private:
  std::FILE* out_;
};

/// @brief Helper class for writing signal data into PCM encoded wave files
///
/// Usage:
///
/// debug::WaveFileWriter writer("out.wav");
/// writer.PushBuffer(&data[1], data.size() - 1);
/// writer.Close();
///
/// This is clearly not optimized at all nor really clean,
/// for debugging purpose only!
class WaveFileWriter {
public:
  explicit WaveFileWriter(const std::string& filename,
                          uint32_t sample_rate = 96000,
                          uint16_t channels_count = 1)
      : out_(std::fopen(filename.c_str(), "wb+")),
        header_({ htonl(0x52494646), // = "RIFF"
                  0,
                  htonl(0x57415645), // = "WAVE"
                  htonl(0x666d7420), // = "fmt "
                  16,
                  static_cast<uint16_t>(1),
                  channels_count,
                  sample_rate,
                  sample_rate * channels_count * 2,
                  static_cast<uint16_t>(channels_count * 2),
                  16,
                  htonl(0x64617461), // = "data"
                  0 }) {
    std::fwrite(&header_, sizeof(Header), 1, out_);
  }
  ~WaveFileWriter() {
    Close();
  }

  /// @brief Push one sample into file writer
  void Push(const Sample sample) {
    float tmp[4];
    VectorMath::Store(&tmp[0], sample);
    PushBuffer(&tmp[0], soundtailor::SampleSize);
  }

  /// @brief Push version for one actual value
  void PushFloat(const float sample) {
    PushBuffer(&sample, 1);
  }

  /// @brief Push version for an entire buffer
  void PushBuffer(const float* buffer, const unsigned int buffer_count) {
    int16_t* out_pcm = new int16_t[buffer_count];
    SOUNDTAILOR_ASSERT(out_pcm);
    for (unsigned int i = 0; i < buffer_count; ++i) {
      int16_t* out(out_pcm + i);
      const float sample(Math::Clamp(buffer[i], -1.0f, 1.0f) * 32767.0f);
      const int16_t tmp_short(static_cast<int16_t>(sample));
      *out = tmp_short;
    }
    header_.subchunk2_size_ += buffer_count * header_.block_align_;
    const unsigned int remainder = sizeof(Header) - offsetof(Header, format_);
    header_.chunk_size_ = header_.subchunk2_size_ + remainder;

    std::fwrite(out_pcm, sizeof(int16_t), buffer_count, out_);
    delete[] out_pcm;
  }

  /// @brief Close writer (automatically done when destroyed)
  void Close(void) {
    if (out_) {
      // Fix header chunk sizes
      std::fseek(out_, offsetof(Header, chunk_size_), SEEK_SET);
      std::fwrite(&header_.chunk_size_, sizeof(uint32_t), 1, out_);

      std::fseek(out_, offsetof(Header, subchunk2_size_), SEEK_SET);
      std::fwrite(&header_.subchunk2_size_, sizeof(uint32_t), 1, out_);

      std::fclose(out_);
      out_ = nullptr;
    }
  }

private:
  static inline bool IsLittleEndian() {
    union {
      uint32_t unsigned_32b_;
      char memory_[4];
    } MemoryMapping;
    MemoryMapping.unsigned_32b_ = 0x01234567;
    return MemoryMapping.memory_[0] == 0x67;
  }

  /// @brief Equivalent to gcc __bswap_32
  static inline uint32_t ByteSwap32(uint32_t value) {
    return ((value << 24) & 0xff000000)
           | ((value << 8) & 0xff0000)
           | ((value >> 8) & 0xff00)
           | ((value >> 24) & 0xff );
  }

  /// @brief Equivalent of Posix or Winsock2's htonl
  static inline uint32_t htonl(uint32_t value)
  {
    return IsLittleEndian() ? ByteSwap32(value) : value;
  }

  struct Header {
    uint32_t chunk_id_;
    uint32_t chunk_size_;
    uint32_t format_;
    uint32_t subchunk1_id_;
    uint32_t subchunk1_size_;
    uint16_t audio_format_;
    uint16_t channels_count_;
    uint32_t sample_rate_;
    uint32_t byte_rate_;
    uint16_t block_align_;
    uint16_t bits_per_sample_;
    uint32_t subchunk2_id_;
    uint32_t subchunk2_size_;
  };

  std::FILE* out_;
  Header header_;
};

}  // namespace debug

#if _COMPILER_MSVC
#pragma warning(pop)
#endif  // _COMPILER_MSVC

#endif  // SOUNDTAILOR_TESTS_DEBUGS_H_
