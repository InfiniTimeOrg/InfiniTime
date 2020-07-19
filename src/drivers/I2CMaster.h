#pragma once

#include <nrfx_twi.h>

namespace Pinetime {
    namespace Drivers {
        class I2CMaster {
        public:
            struct Parameters {

            };
            I2CMaster(/*TODO: const Parameters &parameters*/);
            I2CMaster(const I2CMaster&) = delete;
            I2CMaster& operator=(const I2CMaster&) = delete;
            I2CMaster(I2CMaster&&) = delete;
            I2CMaster& operator=(I2CMaster&&) = delete;

            void Init();
            void Sleep();
            void Wakeup();

            void Read(const uint8_t address, uint8_t *buffer, size_t size);

        private:
            nrfx_twi_t twi;
            nrfx_twi_config_t config;
        };
    }
}