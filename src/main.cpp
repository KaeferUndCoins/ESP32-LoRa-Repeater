#include <Arduino.h>
#include <SPI.h>
// GPS conected to
// RXD -> 17
// TXD -> 16
// GND -> GND
// VCC -> 5V
// PPS -> NC
#include <LoRa.h>
#include <TinyGPS++.h>

#define myID 2
//define the pins used by the transceiver module
#define ss_pin 18 //5
#define rst_pin 14
#define dio0_pin 2
//#define ADC_1 36
#define ledpin 25
int counter = 0;

TinyGPSPlus gps;

void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);
  while (!Serial)
    ;
  Serial.println("LoRa Repeater");
  //setup LoRa transceiver module
  LoRa.setPins(ss_pin, rst_pin, dio0_pin);
  //replace the LoRa.begin(---E-) argument with your location's frequency
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  pinMode(ledpin, OUTPUT);
  int c = 0;
  while (!LoRa.begin(868E6))
  {
    Serial.print(".");
    c++;
    if (c >= 50)
    {
      c = 0;
      Serial.println(".");
    }
    delay(1000);
  }
  // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  digitalWrite(ledpin, HIGH);
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }
  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }
  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }
  Serial.println();
}

void loop()
{
  while (Serial2.available())
  {
    if (gps.encode(Serial2.read()))
      // get the new byte:
      // char inChar = "W";//char(1);//(char)Serial2.read();
      //  displayInfo();
      if (millis() > 5000 && gps.charsProcessed() < 10)
      {

        Serial.println(F("No GPS detected: check wiring."));
        //while (true)
        ;
      }
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    String LoRaData = "";
    // received a packet
    Serial.print("Received packet '");
    // read packet
    while (LoRa.available())
    {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }
    // print RSSI of packet
    Serial.print("' with RSSI ");
    int rssi_rx = LoRa.packetRssi();
    Serial.println(rssi_rx);
    Serial.print("Sending packet: ");
    Serial.println(counter);
    //Send LoRa packet to receiver
    LoRa.beginPacket();
    // LoRa.print("Repeated Message Nr: ");
    //LoRa.print("Payload:|");
    LoRa.print(LoRaData);
    LoRa.print("| Recieved with RSSI: ");
    LoRa.print(rssi_rx);
    LoRa.print("| Resent from Sender: ");
    LoRa.print(myID);
    LoRa.print("| MsgNr:");
    LoRa.print(counter);
    if (gps.location.isValid())
    {
      LoRa.print("| Loaction: ");
      LoRa.print(gps.location.lat());
      LoRa.print(",");
      LoRa.print(gps.location.lng());
    }
    Serial.println(LoRa.endPacket());
    counter++;
    if (counter >= 10000)
    {
      counter = 0;
    }
  }

  if (Serial.available() > 0)
  {
    // read the incoming byte:
    char incomingByte = Serial.read();
    // say what you got:
    Serial.print(incomingByte, DEC);
    Serial.println(" on Serial 0");
  }
}