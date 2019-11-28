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

#ifndef rfquack_config_h
#define rfquack_config_h

#include "config/general.h"
#include "config/logging.h"
#include "config/transport.h"
#include "config/network.h"

#ifndef RFQUACK_RADIO_MAX_MSG_LEN
#define RFQUACK_RADIO_MAX_MSG_LEN 128
#endif

#ifndef RFQUACK_RADIO_TX_QUEUE_LEN
#define RFQUACK_RADIO_TX_QUEUE_LEN 10
#endif

#endif
