#ifndef RFQUACK_PROJECT_RFQNRF24_H
#define RFQUACK_PROJECT_RFQNRF24_H

#include "RadioLibWrapper.h"
#include "rfquack_logging.h"

class RFQnRF24 : public RadioLibWrapper<nRF24> {
public:
    RFQnRF24(Module *module) : RadioLibWrapper(module, "nRF24") {}

    virtual int16_t transmitMode() override {
      // Set up TX_ADDR to last used.
      RFQUACK_LOG_TRACE(F("Setting up TX pipe."))
      int16_t state = nRF24::setTransmitPipe(_addr);
      if (state != ERR_NONE) return state;

      return RadioLibWrapper::transmitMode();
    }

    bool isTxChannelFree() override {
      // Check if max number of retransmits has occurred.
      if (nRF24::getStatus(NRF24_MAX_RT)) {
        nRF24::standby();
        nRF24::clearIRQ();
        RFQUACK_LOG_TRACE(F("No ACK received from previous TX. Abort TX."))
        return true;
      }

      // Call to base method.
      return RadioLibWrapper::isTxChannelFree();
    }

    virtual int16_t receiveMode() override {
      if (_mode != rfquack_Mode_RX) {
        // Set up receiving pipe.
        // Note: this command will bring radio back to standby mode.
        int16_t state = nRF24::setReceivePipe(0, _addr);
        if (state != ERR_NONE)
          return state;

        // Call to base method.
        return RadioLibWrapper::receiveMode();
      } else {
        return ERR_NONE;
      }
    }

    int16_t readData(uint8_t *data, size_t len) override {
      // set mode to standby
      size_t length = 32;

      // read packet data
      SPIreadRxPayload(data, length);

      // add terminating null
      data[length] = 0;

      // clear status bits
      setFlag(false);
      _mod->SPIsetRegValue(NRF24_REG_STATUS, NRF24_RX_DR | NRF24_TX_DS | NRF24_MAX_RT, 6, 4);

      return (ERR_NONE);
    }

    // NOTE: nRF24 does not have a "setSyncword()" method since it's called "address" and is set
    // per pipe.
    int16_t setSyncWord(uint8_t *bytes, pb_size_t size) override {
      // Note: this command will bring radio back to standby mode.
      _mode = rfquack_Mode_IDLE;

      // First try to set addr width.
      int16_t result = nRF24::setAddressWidth(size);
      if (result != ERR_NONE) {
        return result;
      }

      // If addr width is valid store it.
      memcpy(_addr, bytes, size);

      return ERR_NONE;
    }

    // Wrap base method since it changes radio mode.
    int16_t setOutputPower(uint32_t txPower) override {
      _mode = rfquack_Mode_IDLE;
      return nRF24::setOutputPower(txPower);
    }

    // Wrap base method since it changes radio mode and unit of measure (Mhz, Hz)
    int16_t setFrequency(float carrierFreq) override {
      // _mode = rfquack_Mode_IDLE;
      auto freq = (int16_t) carrierFreq;
      RFQUACK_LOG_TRACE("Frequency = %d", freq)
      return nRF24::setFrequency(freq);
    }

    int16_t setFrequencyDeviation(float freqDev) override {
      return RadioLibWrapper::setFrequencyDeviation(freqDev);
    }

    int16_t setCrcFiltering(bool crcOn) override {
      return nRF24::setCrcFiltering(crcOn);
    }

    int16_t setBitRate(float br) override {
      return nRF24::setDataRate(br); // !?!
    }

    int16_t setPromiscuousMode(bool isEnabled) override {
      // Disables CRC and auto ACK.
      int16_t state = setCrcFiltering(!isEnabled);

      // Set address to 0x00AA. (mind the endianness)
      byte syncW[5] = {0xAA, 0x00};
      state |= setSyncWord(syncW, 2);

      // Set fixed packet len
      state |= fixedPacketLengthMode(32);

      return state;
    }

    int16_t fixedPacketLengthMode(uint8_t len) override {
      // Packet cannot be longer than 32 bytes.
      if (len > 32) return ERR_PACKET_TOO_LONG;

      // Turn off Dynamic Payload Length as Global feature
      int16_t status = _mod->SPIsetRegValue(NRF24_REG_FEATURE, NRF24_DPL_OFF, 2, 2);
      RADIOLIB_ASSERT(status);

      // Set len in RX_PW_P0/1
      status = _mod->SPIsetRegValue(NRF24_REG_RX_PW_P0, len, 5, 0);
      status |= _mod->SPIsetRegValue(NRF24_REG_RX_PW_P1, len, 5, 0);
      return status;
    }

    int16_t variablePacketLengthMode(uint8_t len) override {
      int16_t status = _mod->SPIsetRegValue(NRF24_REG_RX_PW_P0, len, 5, 0);
      status |= _mod->SPIsetRegValue(NRF24_REG_RX_PW_P1, len, 5, 0);
      RADIOLIB_ASSERT(status);

      // Turn on Dynamic Payload Length as Global feature
      status |= _mod->SPIsetRegValue(NRF24_REG_FEATURE, NRF24_DPL_ON, 2, 2);
      RADIOLIB_ASSERT(status);

      // Enable dynamic payloads on all pipes
      return _mod->SPIsetRegValue(NRF24_REG_DYNPD, NRF24_DPL_ALL_ON, 5, 0);
    }

    int16_t isCarrierDetected(bool &isDetected) override {
      // Value is correct 170uS after RX mode is issued.
      isDetected = nRF24::isCarrierDetected();
      return ERR_NONE;
    }

    int16_t setAutoAck(bool autoAckOn) override {
      return nRF24::setAutoAck(autoAckOn);
    }

    void
    writeRegister(rfquack_register_address_t reg, rfquack_register_value_t value, uint8_t msb, uint8_t lsb) override {
      _mod->SPIsetRegValue((uint8_t) reg, (uint8_t) value, msb, lsb);
    }

    void removeInterrupts() override {
      nRF24::clearIRQ();
      detachInterrupt(digitalPinToInterrupt(_mod->getIrq()));
    }

    void setInterruptAction(void (*func)(void *)) override {
      attachInterruptArg(digitalPinToInterrupt(_mod->getIrq()), func, (void *) (&_flag), FALLING);
    }

private:
    byte _addr[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7}; // Cannot be > 5 bytes. Default len is 5.
};


#endif //RFQUACK_PROJECT_RFQNRF24_H