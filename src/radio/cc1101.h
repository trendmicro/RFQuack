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

#ifndef rfquack_radio_cc1101_h
#define rfquack_radio_cc1101_h

#include <RadioLib.h>

#define RFQUACK_RADIO "CC1101"

// Modem configuration
#ifndef RFQUACK_RADIO_SYNC_WORDS
#define RFQUACK_RADIO_SYNC_WORDS CC1101_DEFAULT_SYNC_WORD
#endif

#ifndef RFQUACK_RADIO_MAX_MSG_LEN
#define RFQUACK_RADIO_MAX_MSG_LEN CC1101_MAX_PACKET_LENGTH
#endif

#ifdef RFQUACK_RADIO_PIN_CS
#define RFMCC1120_CS RFQUACK_RADIO_PIN_CS
#else
#error "Please define RFQUACK_RADIO_PIN_CS"
#endif

#ifdef RFQUACK_RADIO_PIN_IRQ
#define RFMCC1120_IRQ RFQUACK_RADIO_PIN_IRQ
#else
#error "Please define RFQUACK_RADIO_PIN_IRQ"
#endif

#ifdef RFQUACK_RADIO_PIN_RST
#define RFMCC1120_RST RFQUACK_RADIO_PIN_RST
#else
#error "Please define RFQUACK_RADIO_PIN_RST"
#endif

#define RFQUACK_RADIO_TX_QUEUE_LEN 1

typedef CC1101 RFQRadio;

#endif