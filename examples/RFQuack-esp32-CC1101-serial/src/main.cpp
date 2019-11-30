/*****************************************************************************
 * RFQuack configuration/
 *****************************************************************************/

/* ID definition */
#define RFQUACK_UNIQ_ID "EPS32_CC1101"

/* Transport configuration */
#define RFQUACK_TRANSPORT_SERIAL
#define RFQUACK_SERIAL_BAUD_RATE 115200


/* Radio configuration */
#define RFQUACK_RADIO_PIN_CS 5
#define RFQUACK_RADIO_PIN_GDO0 4
#define RFQUACK_RADIO_PIN_GDO1 22 // GDO1 is not used, can be set to anything.

/* Enable RadioHAL debug messages */
#define RFQUACK_LOG_ENABLED
#define RFQUACK_DEV
#define RADIOLIB_DEBUG

/* Disable Software Serial logging */
#define RFQUACK_LOG_SS_DISABLED

/* Default radio config */
// carrier frequency:                   868.0 MHz
// bit rate:                            4.8 kbps
// Rx bandwidth:                        325.0 kHz
// frequency deviation:                 48.0 kHz
// sync word:                           0xD391

/*****************************************************************************
 * /RFQuack configuration - DO NOT EDIT BELOW THIS LINE (MAYBE YES?)
 *****************************************************************************/

#include "rfquack.h"

RFQCC1101 rfqcc1101(
  new CC1101(
    new Module(RFQUACK_RADIO_PIN_CS, RFQUACK_RADIO_PIN_GDO0, RFQUACK_RADIO_PIN_GDO1)
  )
);


void setup() {
  rfquack_setup(
    &rfqcc1101
  );
}

void loop() {
  rfquack_loop();
}