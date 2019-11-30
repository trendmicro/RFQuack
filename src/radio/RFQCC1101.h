#ifndef RFQUACK_PROJECT_RFQCC1101_H
#define RFQUACK_PROJECT_RFQCC1101_H


#include "RFQRadio.h"
#include "rfquack_logging.h"

class RFQCC1101 : public RFQRadio {
public:
    RFQCC1101(CC1101 *cc1101) : RFQRadio(cc1101) {
      _driver = cc1101;
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
      RFQUACK_LOG_TRACE(F("Entering STANDBY mode."))
      return _driver->standby();
    }


    int16_t transmitMode() override {
      _mode = RFQRADIO_MODE_TX;

      // NOTE: In TX mode the flag is present (true) if TX channel is free to use.
      setFlag(true);
      RFQUACK_LOG_TRACE(F("Entering TX mode."))

      return ERR_NONE;
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
        // Check if timeout has reached. (50mS)
        if (micros() - start >= 50000) {
          RFQUACK_LOG_TRACE(F("Timeout exceeded."))
          break;
        }
      }
      RFQUACK_LOG_TRACE(F("Channel is free, go ahead transmitting"))

      // Remove flag (false), in TX mode this means "TX channel is busy"
      setFlag(false);

      // Start async TX.
      int16_t state = _driver->startTransmit(data, len);
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

      // Start async RX
      int16_t state = _driver->startReceive();
      if (state != ERR_NONE)
        return state;

      // Register an interrupt routine to set flag when radio receives something.
      setFlag(false);
      setInterruptAction(radioInterrupt);

      return ERR_NONE;
    }

    void setInterruptAction(void (*func)(void)) override {
      _driver->setGdo0Action(func);
    }

    int16_t setSyncWord(uint8_t *bytes, pb_size_t size) override {
      if (size == 0) {
        // Warning: as side effect this also disables preamble generation / detection.
        // CC1101 Datasheet states: "It is not possible to only insert preamble or
        // only insert a sync word"
        RFQUACK_LOG_TRACE("Preamble and SyncWord disabled.")
        return _driver->disableSyncWordFiltering();
      }

      // Set syncWord.
      return _driver->setSyncWord(bytes, size);
    }

    int16_t setPreambleLength(uint32_t size) override {
      return _driver->setPreambleLength(size);
    }

    int16_t setFrequency(float carrierFreq) override {
      //This command sets mode to Standby
      _mode = RFQRADIO_MODE_STANDBY;

      return _driver->setFrequency(carrierFreq);
    }

    int16_t setOutputPower(uint32_t txPower) override {
      return _driver->setOutputPower(txPower);
    }

    int16_t fixedPacketLengthMode(uint8_t len) override {
      return _driver->fixedPacketLengthMode(len);
    }

    int16_t variablePacketLengthMode(uint8_t len) override {
      return _driver->variablePacketLengthMode(len);
    }

    int16_t setPromiscuousMode(bool isPromiscuous) override {
      return _driver->setPromiscuousMode(isPromiscuous);
    }

private:
    CC1101 *_driver;
};

#endif //RFQUACK_PROJECT_RFQCC1101_H
