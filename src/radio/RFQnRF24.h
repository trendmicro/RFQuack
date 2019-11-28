#ifndef RFQUACK_PROJECT_RFQNRF24_H
#define RFQUACK_PROJECT_RFQNRF24_H

#include "RFQRadio.h"


class RFQnRF24 : public RFQRadio {
public:
    RFQnRF24(nRF24 *nRf24) : RFQRadio(nRf24) {
      _driver = nRf24;
    }

    int16_t begin() override {
      return _driver->begin();
    }

    uint8_t getPacketLength() override {
      return _driver->getPacketLength();
    }

    int16_t readData(uint8_t *data, size_t len) override {
      // Let's assume readData() call rate is so high that there's
      // always almost a packet to read from radio.
      setIncomingDataAvailable(false);

      return _driver->readData(data, len);
    }

    uint8_t readRegister(uint8_t reg) override {
      return _driver->_mod->SPIreadRegister(reg);
    }

    void writeRegister(uint8_t reg, uint8_t value) override {
      _driver->_mod->SPIwriteRegister(reg, value);
    }

private:
    nRF24 *_driver;
};


#endif //RFQUACK_PROJECT_RFQNRF24_H
