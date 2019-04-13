/**
 * @file configuration.cpp
 * @brief This is the file in which all the static flags are stored.
 * these are to be switched during prototype development.
*/

#include "configuration.h"

/**
* @brief This flag sets whether the ground station should print through the DebugLog via serial.
* Upon prototype finishing, this should be removed along with the debugging functions, this servers as a temporary development flag.
*/
/// Should the ground station output to serial.
bool DEBUG =  true;
/* Should we print debug information through MQTT? (NO) */
bool DEBUG_MQTT = false;

/**
* @brief This flag sets whether the ground station should tune itself from the Function Id "10" transmissions.
* only used for prototyping, if the tuning system fails to allow hardware debugging.
*/
/// Should use the Transceiver settings message to tune its own antenna.
bool AUTOMATIC_TUNING = true;

// see configuration.h
#if defined(RADIOTYPE0)
SX1278 LORA = new LoRa(10, 7, 6);
#endif
#if defined(RADIOTYPE1)
/* Board Arduino UNO */
SX1278 LORA = new LoRa(10,2,6);
#endif
#if defined(RADIOTYPE2)
/* Board Wemos D1 R1 */
RFM95 LORA = new LoRa(16, 15, 15);
#endif
#if defined(RADIOTYPE3)
/* Board NodeMCU 1.0  */
RFM95 LORA = new LoRa(2, 5, 4);
#endif
/* @brief This is the carrier frequency, however, this variable is tuned by the system. Therefore it is not constant.*/
///Transmission frequency measured in MHz (varies).
#if defined(RADIOTYPE0) || defined(RADIOTYPE1)
float CARRIER_FREQUENCY = 434.0f;
#else
float CARRIER_FREQUENCY = 868.50f;
#endif

/* @brief This is the default carrier frequency, this value is constant, used for switching back into wide bandwidth listening modes*/
///Transmission frequency measured in MHz (constant).
#if defined(RADIOTYPE0) || defined(RADIOTYPE1)
float DEFAULT_CARRIER_FREQUENCY = 434.0f;
#else
float DEFAULT_CARRIER_FREQUENCY = 868.50f;
#endif

/* @brief This is the value for the high bandwidth mode.*/
///Transmission bandwidth measured in KHz
float BANDWIDTH = 62.5f;

/* @brief This is the value for the low bandwidth (satellite found) mode.*/
///Transmission bandwidth measured in KHz (varies).
float CONNECTED_BANDWIDTH = 7.8f;

///Chip duration

#if defined(RADIOTYPE0) || defined(RADIOTYPE1)
int SPREADING_FACTOR = 7;  // 12 have trouble with ESP8266, so should be 7 for now MUST BE CHANGED TO 12 FOR REAL GROUNDSTATION
#else
int SPREADING_FACTOR = 7;
#endif

///Code rate
int CODING_RATE = 8; // can be 12.

///Start of transmission frame preamble.
char SYNC_WORD = 0x13;

///Amount of power of radio frequency transmitted
int OUTPUT_POWER = 17; // dBm

///The unique identifyer for this satellite, used as a signature for transmissions.
String TRANSMISSION_SIGNATURE = "FOSSASAT-1";
