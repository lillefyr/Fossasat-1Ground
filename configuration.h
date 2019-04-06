#ifndef CONFIGURATION_H_INCLUDE
#define CONFIGURATION_H_INCLUDE

#include <LoRaLib.h>

extern bool DEBUG;
extern bool DEBUG_MQTT; // send debug messages via MQTT.

extern bool AUTOMATIC_TUNING;

#define RADIOTYPE1

///////////////////////////////////////
// Radiotype for getting the right pins
// and frequencies set in configuration.*
// Arduino does not support mqtt
//
// 0 Original settings
// 1 Arduino + Lora + 434 Mhz
// 2 ESP8266 + Lora + 868.5 Mhz
// 3 NodeMCU 1.0 + Lora + 868.5 Mhz

/////////////////////////
// SX1278 antenna pins //
// ------------------- //
// NSS pin:   7       //
// DIO0 pin:  2        //
// DIO1 pin:  3        //
/////////////////////////


// SX1278 antenna pins RADIOTYPE1//
// ------------------- //
// NSS (CS)  pin:   10       //
// DIO0      pin:  2        //
// DIO1      pin:  6        //
// See http://wiki.dragino.com/index.php?title=Lora_Shield#Pin_Mapping_and_Unused_Pins //
/////////////////////////
#if defined(RADIOTYPE0) || defined(RADIOTYPE1 )
extern SX1278 LORA;
#endif

// NODEMCU antenna pins //
// ------------------- //
// NSS pin:   2        //
// DIO0 pin:  5        //
// DIO1 pin:  4        //
/////////////////////////
// SCK  == GPIO14
// SS   == GPIO2
// MISO == GPIO13
// MOSI == GPIO12
// RST  == GPIO16
// DIO0 == GPIO05   
// DIO1 == GPIO04


/// ESP8266 Hallard node /////////
// DIO0 pin:  15;  // GPIO15 / D8. For the Hallard board shared between DIO0/DIO1/DIO2
// DIO1 pin:  15;  // GPIO15 / D8. Used for CAD, may or not be shared with DIO0
// DIO2 pin:  15;  // GPIO15 / D8. Used for frequency hopping, don't care
// NSS pin:  16;    // GPIO16 / D0. Select pin connected to GPIO16 / D0
//  uint8_t rst=0;    // GPIO 0 / D3. Reset pin not used  
  // MISO 12 / D6
  // MOSI 13 / D7
  // CLK  14 / D5
#if defined(RADIOTYPE2) || defined(RADIOTYPE3)
extern RFM95 LORA;
#endif

///////////////////////////
// LoRa Antenna Settings //
///////////////////////////
extern float DEFAULT_CARRIER_FREQUENCY;
extern float CARRIER_FREQUENCY; // MHz
extern float CONNECTED_BANDWIDTH; // kHz the bandwidt
extern float BANDWIDTH; // kHz
extern int SPREADING_FACTOR;
extern int CODING_RATE; // can be 12.
extern char SYNC_WORD;
extern int OUTPUT_POWER; // dBm

////////////////////////////
// Transmission signature //
////////////////////////////
extern String TRANSMISSION_SIGNATURE;

#endif
