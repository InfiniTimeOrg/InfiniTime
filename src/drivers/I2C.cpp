//
// Created by simon on 7/18/20.
//

#include "I2C.h"

using namespace Pinetime::Drivers;

I2C::I2C(Pinetime::Drivers::I2CMaster &i2cMaster, uint8_t address): i2cMaster{i2cMaster}, address{address} {

}

//TODO: Do we need this?
bool I2C::Init() {
    return false;
}

//TODO: Implement I2C Read Byte
bool I2C::Write(const uint8_t reg, const uint8_t value) {
    return false;
}

//TODO: Implement I2C Read Byte
uint8_t I2C::Read(const uint8_t reg) {
    return 0;
}

void I2C::ReadMemory(uint8_t *buffer, size_t size) {
    i2cMaster.Read(address, buffer, size);
}
