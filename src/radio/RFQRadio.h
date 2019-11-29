#ifndef RFQUACK_PROJECT_RFQRADIO_H
#define RFQUACK_PROJECT_RFQRADIO_H

#define ERR_COMMAND_NOT_IMPLEMENTED -590  // 501 was already taken...
#define ERR_WRONG_MODE              -591

#define RFQRADIO_MODE_STANDBY 0
#define RFQRADIO_MODE_TX 1
#define RFQRADIO_MODE_RX 2

// Enable super powers :)
#define RADIOLIB_GODMODE

#include <RadioLib.h>

// Bad trick to solve IRQ problems. Will be fixed soon.
void *rfqRadioInstance = nullptr;

class RFQRadio {
public:
    /**
     * Initializes radio.
     * @return status_codes
     */
    virtual int16_t begin() {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Puts radio in standby mode.
     * @return
     */
    virtual int16_t standbyMode() {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Puts radio in receive mode and enables RX interrupt.
     * @return
     */
    virtual int16_t receiveMode() {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Puts radio in transmit mode.
     * @return
     */
    virtual int16_t transmitMode() {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Sends packet to air.
     * @param data buffer to send.
     * @param len buffer length.
     * @return
     */
    virtual int16_t transmit(uint8_t *data, size_t len) {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Sets transmitted / received preamble length.
     * @param size size
     * @return
     */
    virtual int16_t setPreambleLength(uint32_t size) {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Sents radio frequency.
     * @param carrierFreq
     * @return
     */
    virtual int16_t setFrequency(float carrierFreq) {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Sets radio output power.
     * @param txPower
     * @return
     */
    virtual int16_t setOutputPower(uint32_t txPower) {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Sets radio syncWord and it's size.
     * @param bytes pointer to syncword.
     * @param size syncword size.
     * @return
     */
    virtual int16_t setSyncWord(uint8_t *bytes, pb_size_t size) {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Returns configured packet len.
     * @return
     */
    virtual uint8_t getPacketLength() = 0;

    /**
     * True whenever there's data available on radio's RX FIFO.
     * @return
     */
    bool isIncomingDataAvailable() {
      // Flag makes sense only if in RX mode.
      if (_mode != RFQRADIO_MODE_RX) {
        return false;
      }

      return _flag;
    }

    /**
     * Consumes data from radio RX FIFO.
     * @param data
     * @param len
     * @return
     */
    virtual int16_t readData(uint8_t *data, size_t len) {
      // Exit if not in RX mode.
      if (_mode != RFQRADIO_MODE_RX) {
        RFQUACK_LOG_TRACE(F("Trying to readData without being in RX mode."))
        return ERR_WRONG_MODE;
      }

      // Let's assume readData() call rate is so high that there's
      // always almost a packet to read from radio.
      setFlag(false);

      // Shame on RadioLib, after readData driver resets interrupts and goes in STANDBY.
      _mode = RFQRADIO_MODE_STANDBY;

      // Read data from fifo.
      return _driver->readData(data, len);
    }

    /**
     * Reads a radio's internal register.
     * @param reg register to read.
     * @return
     */
    virtual uint8_t readRegister(uint8_t reg) = 0;

    /**
     * Writes to a radio's internal register.
     * @param reg register to write.
     * @param value register's value.
     */
    virtual void writeRegister(uint8_t reg, uint8_t value) = 0;

    /**
     * Puts radio in fixed packet length mode.
     * @param len packet's size.
     * @return
     */
    virtual int16_t fixedPacketLengthMode(uint8_t len) {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Puts radio in fixed packet length mode.
     * @param len maximum packet size.
     * @return
     */
    virtual int16_t variablePacketLengthMode(uint8_t len) {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Puts radio in promiscuous mode.
     * @param isPromiscuous
     * @return
     */
    virtual int16_t setPromiscuousMode(bool isPromiscuous) {
      return ERR_COMMAND_NOT_IMPLEMENTED;
    }

    /**
     * Sets callback on interrupt.
     * @param func
     */
    virtual void setInterruptAction(void (*func)(void)) = 0;

    /**
     * Used from IRQ to set flag and from wrapper to clean it.
     * @param flagValue
     */
    void setFlag(bool flagValue) {
      _flag = flagValue;
    }

protected:
    RFQRadio(PhysicalLayer *driver) {
      _driver = driver;
      rfqRadioInstance = this;
    }

    bool getFlag() {
      return _flag;
    }

    uint8_t _mode = RFQRADIO_MODE_STANDBY; // RFQRADIO_MODE_[STANDBY|RX|TX]
private:
    PhysicalLayer *_driver;
    bool _flag = false;
};

void radioInterrupt() {
  if (rfqRadioInstance != nullptr) {
    ((RFQRadio *) rfqRadioInstance)->setFlag(true);
  }
}


#endif //RFQUACK_PROJECT_RFQRADIO_H
