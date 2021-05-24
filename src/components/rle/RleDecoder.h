#pragma once

#include <cstdint>
#include <cstddef>

namespace Pinetime {
  namespace Tools {
    /* 1-bit RLE decoder. Provide the encoded buffer to the constructor and then call DecodeNext() by
     * specifying the output (decoded) buffer and the maximum number of bytes this buffer can handle.
     *
     * Code from https://github.com/daniel-thompson/wasp-bootloader by Daniel Thompson released under the MIT license.
     */
    class RleDecoder {
    public:
      RleDecoder(const uint8_t* buffer, size_t size);
      RleDecoder(const uint8_t* buffer, size_t size, uint16_t foregroundColor, uint16_t backgroundColor);

      void DecodeNext(uint8_t* output, size_t maxBytes);

    private:
      const uint8_t* buffer;
      size_t size;

      size_t encodedBufferIndex = 0;
      int y = 0;
      uint16_t bp = 0;
      uint16_t foregroundColor = 0xffff;
      uint16_t backgroundColor = 0;
      uint16_t color = backgroundColor;
      int processedCount = 0;
    };
  }
}
