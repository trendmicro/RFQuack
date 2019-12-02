#ifndef RFQUACK_PROJECT_RFQNRF24_H
#define RFQUACK_PROJECT_RFQNRF24_H

#include "RFQRadio.h"
#include "rfquack_logging.h"

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

    uint8_t readRegister(uint8_t reg) override {
      return _driver->_mod->SPIreadRegister(reg);
    }

    void writeRegister(uint8_t reg, uint8_t value) override {
      _driver->_mod->SPIwriteRegister(reg, value);
    }

    int16_t standbyMode() override {
      _mode = RFQRADIO_MODE_STANDBY;
      clearInterruptsAndRemoveFlag();
      RFQUACK_LOG_TRACE(F("Entering STANDBY mode."))
      return _driver->standby();
    }

    int16_t transmitMode() override {
      // If we are not coming from TX mode, change mode and set flag (means channel is free)
      if (_mode != RFQRADIO_MODE_TX) {
        _mode = RFQRADIO_MODE_TX;
        _driver->clearIRQ();

        // NOTE: In TX mode the flag is present (true) if TX channel is free to use.
        setFlag(true);
        RFQUACK_LOG_TRACE(F("Entering TX mode."))
      }

      // Set up transmitting pipe.
      // This will automagically setup pipe 1 (?) for TX and pipe 0 for ACKs RX.
      RFQUACK_LOG_TRACE(F("Setting up TX pipe."))
      return _driver->setTransmitPipe(_addr);
    }

    int16_t transmit(uint8_t *data, size_t len) override {
      // Exit if radio is not in TX mode.
      if (_mode != RFQRADIO_MODE_TX) {
        RFQUACK_LOG_TRACE(F("In order to transmit you must be in TX mode."))
        return ERR_WRONG_MODE;
      }

      RFQUACK_LOG_TRACE(F("Request to transmit, checking if channel is free."))
      uint32_t start = micros();

      // If flag is not there, it means that a transmission is occurring.
      while (!getFlag()) {
        // Check if max number of retransmits has occurred.
        if (_driver->getStatus(NRF24_MAX_RT)) {
          _driver->standby();
          _driver->clearIRQ();
          RFQUACK_LOG_TRACE(F("No ACK received from previous TX."))
          break;
        }

        // Check if timeout has reached.
        if (micros() - start >= 60000) {
          _driver->standby();
          _driver->clearIRQ();
          RFQUACK_LOG_TRACE(F("We have been waiting too long for channel to get free."))
          break;
        }
      }
      RFQUACK_LOG_TRACE(F("Channel is free, go ahead transmitting"))

      // Remove flag (false), in TX mode this means "TX channel is busy"
      setFlag(false);

      // Start async TX.
      int16_t state = _driver->startTransmit(data, len, 0); // 3rd param is not used.
      if (state != ERR_NONE)
        return state;

      // Register an interrupt, we'll use it to know when TX is over and channel gets free.
      setInterruptAction(radioInterrupt);

      return ERR_NONE;
    }

    int16_t receiveMode() override {
      // Set mode to RX.
      _mode = RFQRADIO_MODE_RX;
      RFQUACK_LOG_TRACE(F("Entering RX mode."))

      // Set up receiving pipe.
      int16_t state = _driver->setReceivePipe(0, _addr);
      if (state != ERR_NONE)
        return state;

      // Start async RX
      state = _driver->startReceive();
      if (state != ERR_NONE)
        return state;

      // Register an interrupt routine to set flag when radio receives something.
      setFlag(false);
      setInterruptAction(radioInterrupt);

      return ERR_NONE;
    }

    void setInterruptAction(void (*func)(void)) override {
      _driver->setIrqAction(func);
    }


    int16_t setSyncWord(uint8_t *bytes, pb_size_t size) override {
      //Note: this command will bring radio back to standby mode.
      _mode = RFQRADIO_MODE_STANDBY;

      // First try to set addr width.
      int16_t result = _driver->setAddressWidth(size);
      if (result != ERR_NONE) {
        return result;
      }

      // If addr width is valid store it.
      memcpy(_addr, bytes, size);

      return ERR_NONE;
    }

    int16_t setOutputPower(uint32_t txPower) override {
      //Note: this command will bring radio back to standby mode.
      _mode = RFQRADIO_MODE_STANDBY;

      return _driver->setOutputPower(txPower);
    }

    int16_t setFrequency(float carrierFreq) override {
      //Note: this command will bring radio back to standby mode.
      _mode = RFQRADIO_MODE_STANDBY;

      return _driver->setFrequency(carrierFreq);
    }

    int16_t setPromiscuousMode(bool isPromiscuous) override {
      // Set syncWord as preamble's tail.
      byte sync[2] = {0xAA, 0xAA};
      int16_t state = setSyncWord(sync, 2);
      if (state != ERR_NONE) {
        return state;
      }

      // Disable auto ACK
      state = _driver->setAutoAck(false);
      if (state != ERR_NONE) {
        return state;
      }

      // Disable CRC
      return _driver->setCrcFiltering(false);
    }

private:
    void clearInterruptsAndRemoveFlag() {
      _driver->clearIRQ();
      setFlag(false);
    }

    nRF24 *_driver;
    byte _addr[5] = {0x01, 0x23, 0x45, 0x67, 0x89}; // Cannot be > 5 bytes. Default len is 5.
};


#endif //RFQUACK_PROJECT_RFQNRF24_H
