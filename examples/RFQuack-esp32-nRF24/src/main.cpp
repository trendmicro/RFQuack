/*****************************************************************************
 * RFQuack configuration/
 *****************************************************************************/

/* ID definition */
#define RFQUACK_UNIQ_ID "ESP32_nRF24"

/* Network configuration */
#define RFQUACK_NETWORK_ESP32
#define RFQUACK_NETWORK_SSID "SSID"
#define RFQUACK_NETWORK_PASS "PASS"

/* Transport configuration */
#define RFQUACK_TRANSPORT_MQTT
#define RFQUACK_MQTT_BROKER_HOST "192.168.1.104"

/* Radio configuration */
#define RFQUACK_RADIO_PIN_CS 5
#define RFQUACK_RADIO_PIN_CE 4
#define RFQUACK_RADIO_PIN_IRQ 22

/* Enable RadioHAL debug messages */
#define RFQUACK_LOG_ENABLED
#define RFQUACK_DEV
#define RADIOLIB_DEBUG

/* Disable Software Serial logging  */
#define RFQUACK_LOG_SS_DISABLED

/* Default radio config */
// carrier frequency:           2400 MHz
// data rate:                   1000 kbps
// output power:                -12 dBm
// address width:               5 bytes
// address                      0x01, 0x23, 0x45, 0x67, 0x89

/*****************************************************************************
 * /RFQuack configuration - DO NOT EDIT BELOW THIS LINE (MAYBE YES?)
 *****************************************************************************/

#include "rfquack.h"

RFQnRF24 rfqnRF24(
  new nRF24(
    new Module(RFQUACK_RADIO_PIN_CS, RFQUACK_RADIO_PIN_CE, RFQUACK_RADIO_PIN_IRQ)
  )
);


void setup() {
  rfquack_setup(
    &rfqnRF24
  );
}

void loop() {
  rfquack_loop();
}