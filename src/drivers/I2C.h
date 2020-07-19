#pragma once

#include <nrfx_twi.h>
#include "I2CMaster.h"

namespace Pinetime {
    namespace Drivers {
        class I2C {
        public:
            I2C(Drivers::I2CMaster &i2cMaster, uint8_t address);
            I2C(const I2C&) = delete;
            I2C& operator=(const I2C&) = delete;
            I2C(I2C&&) = delete;
            I2C& operator=(I2C&&) = delete;

            bool Init();
            bool Write(const uint8_t reg, const uint8_t value);
            uint8_t Read(const uint8_t reg);
            void ReadMemory(uint8_t* buffer, size_t size);
        private:
            Drivers::I2CMaster &i2cMaster;
            uint8_t address;
        };
    }
}