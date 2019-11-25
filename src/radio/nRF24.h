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
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51 Franklin
 * Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef rfquack_radio_nrf24_h
#define rfquack_radio_nrf24_h

#include <RadioLib.h>

#define RFQUACK_RADIO "nRF24"

// Modem configuration

#ifndef RFQUACK_RADIO_MAX_MSG_LEN
#define RFQUACK_RADIO_MAX_MSG_LEN NRF24_MAX_PACKET_LENGTH
#endif

#ifndef RFQUACK_RADIO_PIN_CS
#error "Please define RFQUACK_RADIO_PIN_CS"
#endif

#ifndef RFQUACK_RADIO_PIN_CE
#error "Please define RFQUACK_RADIO_PIN_CE (Chip Enable)"
#else
#define RFQUACK_RADIO_PIN_IRQ RFQUACK_RADIO_PIN_CE
#endif

#ifndef RFQUACK_RADIO_PIN_IRQ
#error "Please define RFQUACK_RADIO_PIN_IRQ"
#endif

#ifndef RFQUACK_RADIO_PIN_IRQ1
#error "Please define RFQUACK_RADIO_PIN_IRQ1 (nRF24 IRQ)"
#endif

#define RFQUACK_RADIO_TX_QUEUE_LEN 1

#define RFQUACK_RADIO_NO_CUSTOM_PREAMBLE
#define RFQUACK_RADIO_NO_CUSTOM_SYNCWORD
#define RFQUACK_RADIO_NO_CUSTOM_PACKET_FORMAT

typedef nRF24 RFQRadio;

void setInterrupt(RFQRadio rfqRadio, void (*func)(void)){
  rfqRadio.setIrqAction(func);
}

#endif