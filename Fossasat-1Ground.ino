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

#include <Wire.h>

// Payload data from satellite
// BC:1.234;B:2.345;TS:3.456;RC:456;DS:1;
String ID = "00";
String BC = "0.000";
String B = "0.000";
String TS = "0.000";
String RC = "0000";
String DS = "0";

// Local data
String RSSI = "000";
String SNR = "0000";
String ONLINE = "1";
String TUNING = "1";

uint32_t timer;
uint32_t lastOnline;
uint32_t lastTuning;
byte eventRequest = 0;

byte lastByte = 0;

void SendToDisplay(String msg){

  if ( msg.length() == 0 ) { return; }
  if ( msg.length() > 32 ) { Serial.print(F("message too long ")); Serial.println(msg); return; }

  int str_len = msg.length() + 1;
  char char_array[str_len];
  msg.toCharArray(char_array, str_len);

  Wire.beginTransmission(4); // transmit to device #4
  Wire.write(char_array);
  Wire.endTransmission();

  delay(50);
  Serial.print("Send to screen <");
  Serial.print(msg);
  Serial.println(">");
}


void receiveEvent(int bytes) {
// Get event from display ( send ping )
  char c;
  String function_id = "";
  while (Wire.available()) {
    c = Wire.read();
    function_id += c;
  }

  if (function_id == "5") { eventRequest = 5; }
  if (function_id == "7") { eventRequest = 7; }
  if (function_id == "8") { eventRequest = 8; }
  return;
}

/* @brief  startup entry point
  @todo catch failure to SX1278 initializing instead of just locking into a while loop().
*/
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("FOSSASAT-1 GROUNDSTATION"));

  Wire.begin(4);  // join i2c bus (we want 2 way communication)
  Wire.onReceive(receiveEvent);
  
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
  timer = millis() + 10000;  // next display update ( in 10 seconds )
}

void loop() {

  // We use integer array instead of String. Seems to work better.
  uint8_t data[50] = {0};
  for (uint8_t i=0; i<50; i++) { data[i] = 0; }
  int16_t state = LORA.receive(data,48);

  // find data length
  for (lastByte=0; data[lastByte] != 0; lastByte++) { } // Serial.print(data[lastByte]); } Serial.println();
  
  RSSI = String(LORA.getRSSI());
  SNR  = String(LORA.getSNR());
  
  if ( lastByte > 0) {
    lastOnline = millis();
    ONLINE = "0";

    // extract data
    // FOSSASAT-1    // signature is 10 long, skip
    //           9;  // ID
    //             BC:0.00;B:1.65;TS:0.00;RC:150;DS:0; // payload for ID 9
    int indexOfS1 = -1;
    if ( data[11] == ';' ) { indexOfS1 = 12; ID = (char)data[10];}
    else
       if ( data[12] == ';' ) { indexOfS1 = 13; ID = (char)data[10]; ID.concat((char)data[11]); }

    if (indexOfS1 > 0 ) {
      SendToDisplay("ID:"+ID);

      ///////////////
      // recieving //
      ///////////////
      /*
      if (ID == "1")
      {
        Communication_ReceivedStartedSignal();
      }
      else if (ID == "2")
      {
        Communication_ReceivedStoppedSignal();
      }
      else if (ID == "3")
      {
        Communication_ReceivedTransmittedOnline();
      }
      else if (ID == "4")
      {
        Communication_ReceivedDeploymentSuccess();
      }
      else */if (ID == "6")
      {
        Communication_ReceivedPong();
      }
      else if ( ID == "9" )
      {
        // Payload data from satellite
        // BC:1.234;B:2.345;TS:3.456;RC:456;DS:1;
              
        String field;
        String value;
        // start 0
        // field 1
        // colon 2
        // value 3
        // semicolon 4

        byte fieldOrValue = 0;
        for (byte i=indexOfS1; i < lastByte; i++) {
          
          if ( data[i] == ';' ) {
            fieldOrValue = 4;
            
            if ( field == "BC") { BC = value; }
            if ( field == "B")  { B  = value; }
            if ( field == "TS") { TS = value; }
            if ( field == "RC") { RC = value; }
            if ( field == "DS") { DS = value; }
            field = "";
            value = "";
          } // end of value
          
          if ( fieldOrValue == 3 ) { // append to value
            value = value + (char)data[i];
          }
          if ( fieldOrValue == 2 ) { // initial part of value
            fieldOrValue = 3;
            value = (char)data[i];
          }
          
          if ( data[i] == ':' ) { fieldOrValue = 2; } // end of field

          if ( fieldOrValue == 1 ) { // append to field
            field = field+(char)data[i];
          }

          if ( fieldOrValue == 0 ) { // initial part of field
            fieldOrValue = 1;
            field = (char)data[i];
          }

          if ( fieldOrValue == 4 ) { fieldOrValue = 0; }
        }
      }
      else if (ID == "10")
      {
        // Get the frequency error given by the LORA lib, to offset the carrier by.
        float frequencyError = LORA.getFrequencyError();
        Serial.print(F("frequency error: ")); Serial.println(frequencyError);
/*        Communication_ReceivedTune(frequencyError); */
      }
      else
      {
        Serial.println();
      }
    }
  }
  else
  {
    if (state == ERR_RX_TIMEOUT)
    {
      // timeout occurred while waiting for a packet
      //Debugging_Utilities_DebugLog("Timeout waiting to receive a packet.");

      if (HAS_REDUCED_BANDWIDTH) // we have found the satellite already, lost connection
      {
        Debugging_Utilities_DebugLog("(DISCONNECTED) Switching back to wide bandwidth mode.");

        CARRIER_FREQUENCY = DEFAULT_CARRIER_FREQUENCY;
        HAS_REDUCED_BANDWIDTH = false; // enable tracking trigger.

        LORA.setFrequency(CARRIER_FREQUENCY); // setup lora for wide bandwidth.
        LORA.setBandwidth(BANDWIDTH);
        
        TUNING = "0";
        lastTuning = millis();
      }
      else // have not found the satellite already
      {
        //Debugging_Utilities_DebugLog("(UNFOUND) Satellite not found! Listening on wide bandwidth mode...");
        if (( lastOnline + 20000 ) < millis() ) {
          if ( ONLINE == "0" ) { ONLINE = "1"; }
        }
        if (( lastTuning + 20000 ) < millis() ) {
          if ( TUNING == "0" ) { TUNING = 1;   }
        }
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
      //Debugging_Utilities_DebugLog("SPI write failed");
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
      Debugging_Utilities_DebugLog("ERROR STATE = " + String(state));
    }
  }
  if ( timer < millis() ) {
    SendToDisplay("BC:"+BC);
    SendToDisplay("B:"+B);
    SendToDisplay("TS:"+TS);
    SendToDisplay("RC:"+RC);
    SendToDisplay("DS:"+DS);
    SendToDisplay("RSSI:"+RSSI);
    SendToDisplay("SNR:"+SNR);
    SendToDisplay("ONLINE:"+ONLINE);
    SendToDisplay("TUNING:"+TUNING);
    SendToDisplay("GO:GO"); //update screen

    timer = millis() + 10000; //update every 10 seconds
  }
  if ( eventRequest == 5 ) { Communication_TransmitPing(); }
  if ( eventRequest == 7 ) { Communication_TransmitStopTransmitting(); }
  if ( eventRequest == 8 ) { Communication_TransmitStartTransmitting(); }
  eventRequest = 0;
}
