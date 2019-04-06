/**
  @file Fossasat-1Ground.ino
  @brief This code manages the protocol transmission. It takes raw values
  from the system, packs them into the radio protocol and sends it through the
  SX1278.

  The code can be configured to use RFM95, @ 868 Mhz
  Pinout is set by defining RADIOTYPE

  RADIOTYPE1 is based on 2 Arduinos
  1) Lora Shield (SX1278 434 Mhz)
  2) SPFD508 Shield Display  ( code in Fossasat-1Screen.ino )

  Connection between the two with wire
*/

/*
  Based on
  Name:    ground_station.ino
  Created:  7/23/2018
  Author: Richad Bamford (FOSSA Systems)

  Changed: 6/April/2019
  Author: Asbjorn Riedel
*/

#include "configuration.h"
#include "debugging_utilities.h"
#include "state_machine_declerations.h"
#include "communication.h"

String oldPayload = "No Data";
#include <Wire.h>

#ifndef RADIOTYPE1
#include <credentials.h>
#include <connectwifi.h>
#include <mosquitto.h>
char mosquittoClient[] = "groundstation";
#endif

void SendToDisplay(String msg){
#ifndef RADIOTYPE1

  // Send data to display hosted on second Arduino (SPFD508Shield from Aliexpress)
  String str;
  int str_len;
  while ( msg.length() > 0 ) {
    str = msg.substring(0,32);
    msg = msg.substring(32);
    str_len = str.length() + 1;
    char char_array[str_len];
    str.toCharArray(char_array, str_len);

    Wire.beginTransmission(4); // transmit to device #4
    Wire.write(char_array);
    Wire.endTransmission();
    Serial.println(str);
  }
  delay(50);
  #endif
}

void receiveEvent(int bytes) {
// Get event from display ( send ping )
  char c;
  String function_id = "";
  Serial.println("receiveEvent");
  while (Wire.available()) {
    c = Wire.read();
    function_id += c;
  }
  Serial.println(function_id);

  if (function_id == "5")
  {
    Communication_TransmitPing();
    SendToDisplay(function_id);
  }

  if (function_id == "7")
  {
    Communication_TransmitStopTransmitting();
    SendToDisplay(function_id);
  }

  if (function_id == "8")
  {
    Communication_TransmitStartTransmitting();
    SendToDisplay(function_id);
  }

}

#ifndef RADIOTYPE1
/*
 * Replace the library build-in messageReceived function because
 * #define USE_DEFAULT_MESSAGE_RECEIVED true
 * is not set
 * MQTT is not used for the Arduino Arduino setup (no wifi available)
 */
void messageReceived(String &topic, String &function_id) {
  Serial.println("uplink command: " + topic + " - " + function_id);

  if (function_id == "5")
  {
    Communication_TransmitPing();
    SendToDisplay(function_id);
  }

  if (function_id == "7")
  {
    Communication_TransmitStopTransmitting();
    SendToDisplay(function_id);
  }

  if (function_id == "8")
  {
    Communication_TransmitStartTransmitting();
    SendToDisplay(function_id);
  }

  if ( function_id == "DEBUG = true" ) { DEBUG = true; };
  if ( function_id == "DEBUG = false" ) { DEBUG = false; };
  if ( function_id == "DEBUG_MQTT = true" ) { DEBUG = true; DEBUG_MQTT = true; };
  if ( function_id == "DEBUG_MQTT = false" ) { DEBUG_MQTT = false; };
  if ( function_id == "RESTART" ) { setup(); };
}
#endif

/* @brief  startup entry point
  @todo catch failure to SX1278 initializing instead of just locking into a while loop().
*/
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("FOSSASAT-1 GROUNDSTATION");

  Wire.begin(4);  // join i2c bus (we want 2 way communication)
  Wire.onReceive(receiveEvent);

  
#ifndef RADIOTYPE1
  // No Wifi Available 
  while ( startWifi() > 0 ) { Serial.println("No connection to wifi"); }
  publishMQTT("/fossasat-1/logging", "Groundstation connected");
#endif

  // Initialize the SX1278 interface with default settings.
  // See the PDF reports on previous PocketQube attempts for more info.
  Debugging_Utilities_DebugLog("SX1278 interface :\nCARRIER_FREQUENCY "
                               + String(CARRIER_FREQUENCY) + " MHz\nBANDWIDTH: "
                               + String(BANDWIDTH) + " kHz\nSPREADING_FACTOR: "
                               + String(SPREADING_FACTOR) + "\nCODING_RATE: "
                               + String(CODING_RATE) + "\nSYNC_WORD: "
                               + String(SYNC_WORD) + "\nOUTPUT_POWER: "
                               + String(OUTPUT_POWER));

  byte err_check = LORA.begin(DEFAULT_CARRIER_FREQUENCY, BANDWIDTH, SPREADING_FACTOR, CODING_RATE, SYNC_WORD, OUTPUT_POWER);

  if (err_check == ERR_NONE)
  {
    Debugging_Utilities_DebugLog("SX1278 Online!");
  }
  else
  {
    Debugging_Utilities_DebugLog("SX1278 Error code = 0x" + String(err_check, HEX));
    while (true);
  }
}

void loop() {
  
#ifndef RADIOTYPE1
  loopMQTT();
  subscribeMQTT("/fossasat-1/tosat");
#endif

  uint8_t data[60] = {0};
  for (uint8_t i=0; i<60; i++) {
    data[i] = 0;
  }
  int16_t state = LORA.receive(data,58);

  String str = (char *)data;
  
  if ((str.length() > 0) && ( oldPayload != str )) {

#ifndef RADIOTYPE1
    publishMQTT("/fossasat-1/fromsat", str);
#endif
    Debugging_Utilities_DebugLog(str);

    oldPayload = str;

    String RSSI = String(LORA.getRSSI());
    String SNR  = String(LORA.getSNR());
    String SendIt = "RS:" + RSSI;
    SendToDisplay(SendIt);
    SendIt = "SN:" + SNR;
    SendToDisplay(SendIt);

    SendToDisplay("ONLINE: ");

    // Added protocol entry to get RSSI and SNR from groundstation to the display
    SendIt = "FOSSASAT-120;RS:" + RSSI + ";SN:" + SNR + ";";
    
#ifndef RADIOTYPE1
    publishMQTT("/fossasat-1/fromsat", SendIt);
#endif

    Debugging_Utilities_DebugLog(SendIt);
   
    String signature = str.substring(0, 10);
    String withoutSignature = str.substring(10);

    int indexOfS1 = withoutSignature.indexOf(';');
    String message = withoutSignature.substring(indexOfS1 + 1);

    String function_id = withoutSignature.substring(0, indexOfS1);

    SendIt = "ID:" + function_id;
    SendToDisplay(SendIt);
    if (function_id == "9")
    {
      // Payload data from satellite
      // BC:1.234;B:2.345;TS:3.456;RC:456;DS:1;
      String data;
      while ( message.length() > 0) {
        // Split in fields and send one by one
        indexOfS1 = message.indexOf(';');
        data = message.substring(0, indexOfS1);

        message = message.substring(indexOfS1+1, message.length() );
        SendToDisplay(data);
        
#ifndef RADIOTYPE1
        // Needed for esp8266
        yield();
#endif
      }
    }
  }
  else
  {
    if (state == ERR_RX_TIMEOUT)
    {
      // timeout occurred while waiting for a packet
      Debugging_Utilities_DebugLog("Timeout waiting to receive a packet.");

      if (HAS_REDUCED_BANDWIDTH) // we have found the satellite already, lost connection
      {
        Debugging_Utilities_DebugLog("(DISCONNECTED) Switching back to wide bandwidth mode.");

        CARRIER_FREQUENCY = DEFAULT_CARRIER_FREQUENCY;
        HAS_REDUCED_BANDWIDTH = false; // enable tracking trigger.

        LORA.setFrequency(CARRIER_FREQUENCY); // setup lora for wide bandwidth.
        LORA.setBandwidth(BANDWIDTH);
        
        SendToDisplay("TUNINGON: ");
      }
      else // have not found the satellite already
      {
        Debugging_Utilities_DebugLog("(UNFOUND) Satellite not found! Listening on wide bandwidth mode...");
        
        SendToDisplay("OFFLINE: ");
        SendToDisplay("TUNINGOFF: ");
      }
    }
    else if (state == ERR_CRC_MISMATCH)
    {
      // packet was received, but is malformed
      Debugging_Utilities_DebugLog("Packet has been received but malformed, CRC error,");
    }
    else if (state == ERR_UNKNOWN)
    {
      Debugging_Utilities_DebugLog("Unknown error occured!");
    }
    else if (state == ERR_CHIP_NOT_FOUND)
    {
      Debugging_Utilities_DebugLog("Could not find chip.");
    }
    else if (state == ERR_EEPROM_NOT_INITIALIZED)
    {
      Debugging_Utilities_DebugLog("EEPROM not initialized");
    }
    else if (state == ERR_PACKET_TOO_LONG)
    {
      Debugging_Utilities_DebugLog("Packet too long...");
    }
    else if (state == ERR_TX_TIMEOUT)
    {
      Debugging_Utilities_DebugLog("TX Timout.");
    }
    else if (state == ERR_RX_TIMEOUT)
    {
      Debugging_Utilities_DebugLog("RX Timeout.");
    }
    else if (state == ERR_INVALID_BANDWIDTH)
    {
      Debugging_Utilities_DebugLog("Invalid bandwidth.");
    }
    else if (state == ERR_INVALID_SPREADING_FACTOR)
    {
      Debugging_Utilities_DebugLog("Invalid spreading factor.");
    }
    else if (state == ERR_INVALID_CODING_RATE)
    {
      Debugging_Utilities_DebugLog("Invalid coding rate.");
    }
    else if (state == ERR_INVALID_BIT_RANGE)
    {
      Debugging_Utilities_DebugLog("Invalid bit range.");
    }
    else if (state == ERR_INVALID_FREQUENCY)
    {
      Debugging_Utilities_DebugLog("Invalid frequency.");
    }
    else if (state == ERR_INVALID_OUTPUT_POWER)
    {
      Debugging_Utilities_DebugLog("Invalid output power.");
    }
    else if (state == PREAMBLE_DETECTED)
    {
      Debugging_Utilities_DebugLog("Preamble detected.");
    }
    else if (state == CHANNEL_FREE)
    {
      Debugging_Utilities_DebugLog("Channel free.");
    }
    else if (state == ERR_SPI_WRITE_FAILED)
    {
      Debugging_Utilities_DebugLog("SPI write failed");
    }
    else if (state == ERR_INVALID_CURRENT_LIMIT)
    {
      Debugging_Utilities_DebugLog("Invalid current limit");
    }
    else if (state == ERR_INVALID_PREAMBLE_LENGTH)
    {
      Debugging_Utilities_DebugLog("Invalid preamble length");
    }
    else if (state == ERR_INVALID_GAIN)
    {
      Debugging_Utilities_DebugLog("Invalid gain.");
    }
    else
    {
      //Debugging_Utilities_DebugLog("ERROR STATE = " + String(state));
    }
  }
}
