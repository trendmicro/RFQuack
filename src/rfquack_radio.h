/*
 * RFQuack is a versatile RF-hacking tool that allows you to sniff, analyze, and
 * transmit data over the air. Consider it as the modular version of the great
 *
 * Copyright (C) 2019 Trend Micro Incorporated
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef rfquack_radio_h
#define rfquack_radio_h

#include "rfquack_common.h"
#include "rfquack_logging.h"
#include "rfquack_transport.h"

#include "rfquack.pb.h"

#include <cppQueue.h>
#include "RadioLib.h"
#include "radio/cc1101.h"

/*****************************************************************************
 * Variables
 *****************************************************************************/
Queue rfquack_rx_q(sizeof(rfquack_Packet), RFQUACK_RADIO_RX_QUEUE_LEN, FIFO,
                   true);

/* Radio instance */
// TODO use RFQUACK_RADIO_PIN_RST
#define RFQUACK_RADIO_PIN_IRQ1 7//TODO FIX!
Module *_mod = new Module(RFQUACK_RADIO_PIN_CS, RFQUACK_RADIO_PIN_IRQ, RFQUACK_RADIO_PIN_IRQ1);
RFQRadio rfquack_rf(_mod);

volatile bool _incomingDataAvailable = false;

/**
 * Function called when a complete packet is received
 */
void radioInterrupt(void) {
  _incomingDataAvailable = true;
}

/**
 * @brief Changes the radio state in the radio driver.
 */
void rfquack_update_mode() {
  if (rfq.mode == rfquack_Mode_IDLE) {
    int16_t result = rfquack_rf.standby();
    RFQUACK_LOG_TRACE("Radio in IDLE mode, resultCode=%d", result)
  }

  if (rfq.mode == rfquack_Mode_REPEAT || rfq.mode == rfquack_Mode_RX) {
    int16_t result = rfquack_rf.startReceive();

    //Set interrupt on new packet
    _incomingDataAvailable = false;
    rfquack_rf.setGdo0Action(radioInterrupt);

    RFQUACK_LOG_TRACE("Radio in RECEIVE mode, resultCode=%d", result)
  }

  // never set TX mode, it happens automatically
}

/**
 * @brief Changes preamble length in the radio driver.
 */
void rfquack_update_preamble() {
  int16_t result = rfquack_rf.setPreambleLength(rfq.modemConfig.preambleLen);
  RFQUACK_LOG_TRACE("Preamble length:     %d bytes, resultCode=%d", rfq.modemConfig.preambleLen, result)
}

/**
 * @brief Changes frequency in the radio driver.
 */
void rfquack_update_frequency() {
  int16_t result = rfquack_rf.setFrequency(rfq.modemConfig.carrierFreq);
  RFQUACK_LOG_TRACE("Frequency :     %f Mhz (?), resultCode=%d", rfq.modemConfig.carrierFreq, result)
}

/**
 * @brief Changes TX power in the radio driver
 */
void rfquack_update_tx_power() {
  int16_t result = rfquack_rf.setOutputPower(rfq.modemConfig.txPower);
  RFQUACK_LOG_TRACE("Output Power :     %f dBm (?), resultCode=%d", rfq.modemConfig.txPower, result)

#ifdef RFQUACK_RADIO_SET_HIGHPOWER
  Log.fatal("HIGHPOWER mode is not implemented.");
#endif
}

/**
 * @brief Chages modem config choice in the radio driver
 */
void rfquack_update_modem_config_choice() {
  Log.fatal("Canned configs not implemented.");
}

/**
 * @brief Changes sync words in the radio driver.
 */
void rfquack_update_sync_words() {
  if (rfq.modemConfig.syncWords.size > 0) {
    int16_t result = rfquack_rf.setSyncWord((uint8_t *) (rfq.modemConfig.syncWords.bytes),
                                            rfq.modemConfig.syncWords.size);
    RFQUACK_LOG_TRACE("Sync Words:          %d bytes, resultCode=%d", rfq.modemConfig.syncWords.size, result)
  } else {
    uint8_t nil = 0;
    int16_t result = rfquack_rf.setSyncWord(&nil, 0);
    RFQUACK_LOG_TRACE("Sync Words:          None (sync words detection disabled), resultCode=%d", result)
  }
}

/*
 * Setup the radio:
 *
 *      - initialize the driver
 *      - set the carrier frequency
 *      - set the modem registers
 *      - set preamble length and sync words
 */
void rfquack_radio_setup() {

  RFQUACK_LOG_TRACE("📡 Setting up radio (CS: %d, RST: %d, IRQ: %d)",
                    RFQUACK_RADIO_PIN_CS, RFQUACK_RADIO_PIN_RST, RFQUACK_RADIO_PIN_IRQ)

  if (rfquack_rf.begin() != ERR_NONE) {
    Log.fatal("⚠️  Radio init failed");
    return;
  }
  RFQUACK_LOG_TRACE("📶 Radio initialized ")

#ifdef RFQUACK_RADIO_SET_FREQ
  rfquack_update_frequency();
#endif

#ifdef RFQUACK_RADIO_SET_POWER
  rfquack_update_tx_power();
#endif

#ifdef RFQUACK_RADIO_SET_RF
  // TODO NRF24/51/73 call .setRF(datarate, power)
#endif

#ifdef RFQUACK_RADIO_HAS_MODEM_CONFIG
  rfquack_update_modem_config_choice();

  rfquack_update_sync_words();
#ifdef RFQUACK_RADIO_SET_PREAMBLE
  rfquack_update_preamble();
#endif

  Log.trace("Max payload length:  %d bytes", RFQUACK_RADIO_MAX_MSG_LEN);
#endif // RFQUACK_RADIO_HAS_MODEM_CONFIG

  rfquack_update_mode();

  Log.trace("📶 Radio is fully set up (RFQuack mode: %d)", rfq.mode);
}

/**
 * @brief Update queue statistics.
 */
void rfquack_update_radio_stats() {
  rfq.stats.rx_queue = rfquack_rx_q.getCount();
}

/**
 * @brief Enqueue a packet in a given queue.
 *
 * Check if the packet holds no more than the RFQUACK_RADIO_MAX_MSG_LEN bytes,
 * check if the queue has enough room available, and push the packet into it.
 *
 * @param q Pointer to queue
 * @param pkt Packet to be enqueued
 *
 * @return True only if the queue has room and enqueueing went through.
 */
bool rfquack_enqueue_packet(Queue *q, rfquack_Packet *pkt) {
  if (pkt->data.size > sizeof(rfquack_Packet)) {
    Log.error("Cannot enqueue: message length exceeds queue size limit");
    return false;
  }

  if (q->isFull()) {
    Log.error("Cannot enqueue because queue is full:"
              " slow down or increase the queue size!");
    return false;
  }

  q->push(pkt);

  RFQUACK_LOG_TRACE("Packet enqueued: %d bytes", pkt->data.size);

  return true;
}

/**
 * @brief Transmit a packet in the air.
 *
 * Fill a packet buffer with data up to the lenght, set its size to len, and
 * enqueue it on the TX queue for transmission.
 *
 * @param pkt Pointer to a Packet struct
 *
 * @return Wether the transmission was correct
 */
bool rfquack_send_packet(rfquack_Packet *pkt) {
  if (pkt->has_repeat && pkt->repeat == 0) {
    Log.verbose("Zero packet repeat: no transmission");
    return false;
  }

  uint32_t repeat = 1;
  uint32_t correct = 0;

  if (pkt->has_repeat)
    repeat = pkt->repeat;

  for (uint32_t i = 0; i < repeat; i++) {
    int16_t result = rfquack_rf.transmit((uint8_t *) (pkt->data.bytes), pkt->data.size);
    RFQUACK_LOG_TRACE("Packet trasmitted, resultCode=%d", result)

    if (result == ERR_NONE) {
      correct++;
    }

    if (pkt->has_delayMs)
      delay(pkt->delayMs);
  }

  Log.verbose("%d/%d packets transmitted", repeat, correct);

  return true;
}

/**
 * @brief If any packets are in RX queue, send them all via the transport.
 * If in repeat mode, modify and retransmit.
 *
 * We send them one at a time, even if there are more than one in the queue.
 * This is because we rather have a full RX queue but zero packet lost, and
 * we want to give other functions in the loop their share of time.
 */
void rfquack_rx_flush_loop() {
  rfquack_Packet pkt;
  while (rfquack_rx_q.pop(&pkt)) {
    if (rfq.mode == rfquack_Mode_REPEAT) {
      // apply all packet modifications
      rfquack_apply_packet_modifications(&pkt);

      pkt.has_repeat = true;
      pkt.repeat = rfq.tx_repeat_default;

      // send
      rfquack_send_packet(&pkt);

      return;
    }

    uint8_t buf[RFQUACK_MAX_PB_MSG_SIZE];
    pb_ostream_t ostream = pb_ostream_from_buffer(buf, sizeof(buf));

    if (!pb_encode(&ostream, rfquack_Packet_fields, &pkt)) {
      Log.error("Encoding failed: %s", PB_GET_ERROR(&ostream));
    } else {
      if (!rfquack_transport_send(
        RFQUACK_OUT_TOPIC RFQUACK_TOPIC_SEP RFQUACK_TOPIC_PACKET, buf,
        ostream.bytes_written))
        Log.error("Failed sending to transport");
      else RFQUACK_LOG_TRACE("%d bytes of data sent on the transport",
                             ostream.bytes_written);
    }
  }
}

/**
 * @brief Reads any data from the RX FIFO and push it to the RX queue.
 *
 * This is the main receive loop.
 */
void rfquack_rx_loop() {
  if (rfq.mode != rfquack_Mode_RX && rfq.mode != rfquack_Mode_REPEAT)
    return;

  rfquack_Packet pkt;
  uint8_t len = rfquack_rf.getPacketLength();

  if (_incomingDataAvailable) { // Radio sent an interrupt
    int16_t result = rfquack_rf.readData((uint8_t *) pkt.data.bytes, len);
    RFQUACK_LOG_TRACE("Recieved packet, resultCode=%d", result)

    //Remove flag and put radio back in RX mode
    _incomingDataAvailable = false;
    rfquack_rf.startReceive();

    // Update stats
    if (result == ERR_NONE) {
      rfq.stats.rx_packets++;
    } else {
      rfq.stats.rx_failures++;
    }

    // Fill missing data
    pkt.data.size = len;
    pkt.millis = millis();
    pkt.has_millis = true;

    //Apply packet filters
    if (rfquack_packet_filter(&pkt)) {
      //Put packet in incoming queue.
      rfquack_enqueue_packet(&rfquack_rx_q, &pkt);

#ifdef RFQUACK_DEV
      rfquack_log_packet(&pkt);
#endif
    }
  }
}

/*
 * Change modem config choice
 */
void rfquack_change_modem_config_choice(uint32_t index) {

  // TODO input validation

  Log.trace("Modem config choice index: %d -> %d",
            rfq.modemConfig.modemConfigChoiceIndex, index);

  rfq.modemConfig.modemConfigChoiceIndex = index;
  rfquack_update_modem_config_choice();
}

/**
 * @brief Read register value (if permitted by the radio driver).
 *
 * @param addr Address of the register
 *
 * @return Value from the register.
 */
uint8_t rfquack_read_register(uint8_t reg) {
  return _mod->SPIreadRegister(reg);
}

/**
 * @brief Write register value
 *
 * @param addr Address of the register
 *
 */
void rfquack_write_register(uint8_t reg,
                            uint8_t value) {
  _mod->SPIwriteRegister(reg, value);
}

/**
 * @brief Sets the packet format (fixed or variable), and its length.
 *
 */
static void rfquack_set_packet_format(char *payload, int payload_length) {
  // init
  rfquack_PacketFormat fmt;

  // create stream from buffer
  pb_istream_t istream =
    pb_istream_from_buffer((uint8_t *) payload, payload_length);

  if (!pb_decode(&istream, rfquack_PacketFormat_fields, &fmt)) {
    Log.error("Cannot decode PacketFormat: %s", PB_GET_ERROR(&istream));

    return;
  }

  if (fmt.fixed) {
    int16_t result = rfquack_rf.fixedPacketLengthMode((uint8_t) fmt.len);
    RFQUACK_LOG_TRACE("Setting radio to fixed len of %d bytes, resultCode=%d", (uint8_t) fmt.len, result)
  } else {
    int16_t result = rfquack_rf.variablePacketLengthMode((uint8_t) fmt.len);
    RFQUACK_LOG_TRACE("Setting radio to variable len ( max %d bytes), resultCode=%d", (uint8_t) fmt.len, result)
  }
}

/*
 * Change TX power
 */
void rfquack_change_tx_power(uint32_t txPower) {

  // TODO input validation

  Log.trace("TX power: %d -> %d", rfq.modemConfig.txPower, txPower);

  rfq.modemConfig.txPower = txPower;
  rfquack_update_tx_power();
}

void rfquack_set_promiscuous(bool promiscuous) {
  int16_t result = rfquack_rf.setPromiscuousMode(promiscuous);
  RFQUACK_LOG_TRACE("Promiscuous mode set to %d, resultCode=%d", promiscuous, result)
}

/*
 * Change sync words
 */
void rfquack_change_sync_words(rfquack_ModemConfig_syncWords_t syncWords) {
  // input validation
  if (syncWords.size == 0) {
    rfq.modemConfig.syncWords.size = 0;
    Log.trace("Disabling sync words detection");
    rfquack_update_sync_words();
    return;
  }

  for (uint8_t i = 0; i < syncWords.size; i++)
    if (syncWords.bytes[i] == 0x00) {
      Log.warning("syncWords[%d] = 0x00, which is disallowed. Not changing!",
                  i);
      return;
    }

  memcpy(rfq.modemConfig.syncWords.bytes, syncWords.bytes, syncWords.size);
  rfq.modemConfig.syncWords.size = syncWords.size;
  rfquack_update_sync_words();
}

/*
 * Change preamble len
 */
void rfquack_change_preamble_len(size_t preambleLen) {
  // TODO input validation

  RFQUACK_LOG_TRACE("Preamble length: %d -> %d", rfq.modemConfig.preambleLen, preambleLen)

  rfq.modemConfig.preambleLen = preambleLen;
  rfquack_update_preamble();
}

/*
 * Change is high power module
 */
void rfquack_change_is_high_power_module(bool isHighPowerModule) {
  // TODO input validation

  RFQUACK_LOG_TRACE("Is high power: %d -> %d", rfq.modemConfig.isHighPowerModule, isHighPowerModule);

  rfq.modemConfig.isHighPowerModule = isHighPowerModule;
  rfquack_update_tx_power();
}

/*
 * Change carrier frequency
 */
void rfquack_change_carrier_freq(float carrierFreq) {
  // TODO input validation

  RFQUACK_LOG_TRACE("Carrier frequency: %F -> %F", rfq.modemConfig.carrierFreq, carrierFreq);

  rfq.modemConfig.carrierFreq = carrierFreq;
  rfquack_update_frequency();
}

uint32_t rfquack_set_modem_config(rfquack_ModemConfig *modemConfig) {
  rfquack_ModemConfig _c = *modemConfig;
  uint32_t changes = 0;

  RFQUACK_LOG_TRACE("Changing modem configuration");

  if (_c.has_carrierFreq) {
    rfquack_change_carrier_freq(_c.carrierFreq);
    changes++;
  }

  if (_c.has_txPower) {
    rfquack_change_tx_power(modemConfig->txPower);
    changes++;
  }

  if (_c.has_isHighPowerModule) {
    rfquack_change_is_high_power_module(_c.isHighPowerModule);
    changes++;
  }

  if (_c.has_preambleLen) {
    rfquack_change_preamble_len(_c.preambleLen);
    changes++;
  }

  if (_c.has_syncWords) {
    rfquack_change_sync_words(_c.syncWords);
    changes++;
  }

  if (_c.has_modemConfigChoiceIndex) {
    rfquack_change_modem_config_choice(modemConfig->modemConfigChoiceIndex);
    changes++;
  }

  return changes;
}

#endif
