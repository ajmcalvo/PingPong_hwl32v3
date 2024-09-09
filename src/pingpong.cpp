/*
   RadioLib SX126x Ping-Pong Example

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/

   Modified by: @amontero at Telegram
                ajmcalvo at GitHub
*/

// include the library
#include <RadioLib.h>

// uncomment the following only on one
// of the nodes to initiate the pings
#define INITIATING_NODE

// Defaults for heltec_wifi_lora_32_v3
// NSS pin:        8
// DIO1 pin:      14
// NRST pin:      12
// BUSY pin:      13
// LORA MOSI pin: 10
// LORA MISO pin: 11
// LORA SCK pin:   9

#define LORA_NSS 8
#define LORA_DIO1 14
#define LORA_NRST 12
#define LORA_BUSY 13

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);

// save transmission states between loops
int transmissionState = RADIOLIB_ERR_NONE;

// flag to indicate transmission or reception state
bool transmitFlag = false;

// flag to indicate that a packet was sent or received
volatile bool operationDone = false;

// this function is called when a complete packet
// is transmitted or received by the module
// IMPORTANT: this function MUST be 'void' type
//            and MUST NOT have any arguments!
void setFlag(void) {
  // we sent or received a packet, set the flag
  operationDone = true;
}

void setup() {
  Serial.begin(115200);

//  if needed
//  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);

  radio.setFrequency(865.0F);
  radio.setSpreadingFactor(7U);
  radio.setCodingRate(5U);
  radio.setOutputPower(1);

  // initialize SX1262 with default and configured settings
  log_d("[SX1262] Initializing ... ");
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    log_d("\t\tInitialization success!");
  } else {
    log_d("\t\tInitialization failed, code %d", state);
    while (true);
  }

  // set the function that will be called
  // when new packet is received
  radio.setDio1Action(setFlag);

  #if defined(INITIATING_NODE)
    // send the first packet on this node
    log_d("[SX1262] Sending first packet ... ");
    transmissionState = radio.startTransmit("Hello World!");
    transmitFlag = true;
  #else
    // start listening for LoRa packets on this node
    log_d("[SX1262] Starting to listen ... "));
    state = radio.startReceive();
    if (state == RADIOLIB_ERR_NONE) {
      log_d("\t\tReception success!");
    } else {
      log_d("\t\tReception failed, code %d", state);
      while (true);
    }
  #endif
}

void loop() {
  // check if the previous operation finished
  if(operationDone) {
    // reset flag
    operationDone = false;

    if(transmitFlag) {
      // the previous operation was transmission, listen for response
      // print the result
      if (transmissionState == RADIOLIB_ERR_NONE) {
        // packet was successfully sent
        log_d("\t\tTransmission finished!");

      } else {
        log_d("\t\tTransmission failed, code %d", transmissionState);
      }

      // listen for response
      radio.startReceive();
      transmitFlag = false;

    } else {
      // the previous operation was reception
      // print data and send another packet
      String str;
      int state = radio.readData(str);

      if (state == RADIOLIB_ERR_NONE) {
        // packet was successfully received
        log_d("[SX1262] Received packet!");

        // print data of the packet
        log_d("[SX1262] Data:\t\t%s", str);

        // print RSSI (Received Signal Strength Indicator)
        log_d("[SX1262] RSSI:\t\t%.1f dBm",radio.getRSSI());

        // print SNR (Signal-to-Noise Ratio)
        log_d("[SX1262] SNR:\t\t%.1f dB", radio.getSNR());

      }

      // wait a second before transmitting again
      delay(1000);

      // send another one
      log_d("[SX1262] Sending another packet ... ");
      transmissionState = radio.startTransmit("Hello World!");
      transmitFlag = true;
    }
  
  }
}
