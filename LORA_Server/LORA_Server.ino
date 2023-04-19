// LORA_Server
// Feather9x_RX
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messaging client (receiver)
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example Feather9x_TX

#include <SPI.h>
#include <RH_RF95.h>

// for Feather32u4 RFM9x
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED 13

// TMP36 sensor analog input
#define TEMP_INPUT 18 

#define COMMAND_SIZE 4

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(115200);
  while(!Serial) 
  {
    delay(1);
  }
  delay(100);

  Serial.println("LORA_Server starting...");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while(!rf95.init()) 
  {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while(1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if(!rf95.setFrequency(RF95_FREQ)) 
  {
    Serial.println("setFrequency failed");
    while(1);
  }
  Serial.print("Set Freq to: "); 
  Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

String reply;

void loop()
{
  //Serial.println("Waiting for command...");
  if(rf95.available())
  {
    // Should be a message for us now
    //uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t buf[COMMAND_SIZE];
    uint8_t len = sizeof(buf);

    if(rf95.recv(buf, &len))
    {
      digitalWrite(LED, HIGH);
      RH_RF95::printBuffer("Received: ", buf, len);
      //char* command = substr(buf, 0, 4); // WTF not here
      //String command = String((char*)buf).substring(0, COMMAND_SIZE);
      Serial.println("Got a command");
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);

      if(buf[0] == ':') // Commands always start with a ':'
      {
        if(buf[1] == 'A')
        {
          Serial.println("Got an A!!!");
          reply = "Thanks dude";
        }
        else if(buf[1] == 'T')
        {
          Serial.println("Getting temperature...");
          int reading = analogRead(TEMP_INPUT);
          // converting that reading to voltage, for 3.3v arduino use 3.3
          float voltage = reading * 3.3;
          voltage /= 1024.0; 
  
          // now print out the temperature
          float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset to degrees ((voltage - 500mV) times 100)

          Serial.print(temperatureC); Serial.println(" degrees C");

          reply = ":TMP " + String(temperatureC);
        }
        else
        {
          Serial.println("Got an unknown command");
          reply = ":NAK unknown command";
        }
      }
      else
      {
        Serial.println("Got a non-command");
        reply = ":NAK non-command";
      }

      // Send the reply
      rf95.send(reply.c_str(), reply.length());
      rf95.waitPacketSent();
      Serial.println("Sent reply: " + reply);
      digitalWrite(LED, LOW);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
}