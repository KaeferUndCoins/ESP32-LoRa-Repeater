#include <Arduino.h>

  /*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

#include <SPI.h>
#include <LoRa.h>
//#include <SoftwareSerial.h>
//#include <HardwareSerial.h> // sollte bereits mit Arduino IDE installiert sein
//#include <SoftwareSerial.h>
//HardwareSerial SerialGPS(2);
//define the pins used by the transceiver module
#define ss 18//5
#define rst 14
#define dio0 2
#define ADC_1 36 
#define ledpin 25 
int counter = 0;
int c=0;
int test=0;

void setup() {
  //initialize Serial Monitor  g
  Serial.begin(9600);  
  //SerialGPS.begin(9600, SERIAL_8N1, 16, 17);
  while (!Serial);
  Serial.println("LoRa Repeater");
  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  pinMode(ledpin,OUTPUT);  
  while (!LoRa.begin(868E6)) {
    Serial.print(".");
    c++;
    if(c>=50){c=0;Serial.println(".");}
    delay(1000);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  digitalWrite(ledpin,HIGH);
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  pinMode(ADC_1,INPUT);
}

int incomingByte = 0; // for incoming serial data
int sensIN;
void loop() {
 int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String LoRaData="";
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
       LoRaData = LoRa.readString();
      Serial.print(LoRaData); 
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    
    int rssi_rx=LoRa.packetRssi();
Serial.println(rssi_rx);

    
  sensIN=analogRead(ADC_1);
  Serial.print("Sending packet: ");
  Serial.println(counter);
  
  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print("Message Nr: ");
  LoRa.print(counter);
  LoRa.print("  -   RSSI:");
  LoRa.print(sensIN);
  Serial.println(LoRa.endPacket());
  counter++;
  if(counter>=10000){
    counter=0;
  }
  delay(1000);

   
  }







  
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    // say what you got:
   Serial.print(incomingByte, DEC);
     Serial.println(" on Serial 0");
  }

}