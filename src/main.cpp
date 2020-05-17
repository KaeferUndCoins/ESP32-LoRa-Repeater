#include <Arduino.h>

  /*********
  Modified from the examples of the Arduino LoRa library
  More resources: https://randomnerdtutorials.com
*********/

// GPS conected to 
// RXD -> 17
// TXD -> 16
// GND -> GND
// VCC -> 5V
// PPS -> NC

#include <SPI.h>
#include <LoRa.h>
#define myID 2
//define the pins used by the transceiver module
#define ss_pin 18//5
#define rst_pin 14
#define dio0_pin 2
#define ADC_1 36 
#define ledpin 25 
int counter = 0;
int c=0;
int test=0;

void setup() {
  Serial.begin(9600);  
//Serial1.begin(9600); 
Serial2.begin(9600); 

  while (!Serial);
  Serial.println("LoRa Repeater");
  //setup LoRa transceiver module
  LoRa.setPins(ss_pin, rst_pin, dio0_pin);  
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

// for Serial
int incomingByte = 0; // for incoming serial data
String inputString="";
String dataString="";
bool stringComplete=false;
unsigned int dataPos=0;
unsigned int dataPosOld=0; 
String GPS_word="";/*
  GLL - Geographic Position - Latitude/Longitude 
           1       2 3        4 5         6 7 
       |       | |        | |         | | 
$--GLL,llll.ll,a,yyyyy.yy,a,hhmmss.ss,A*hh<CR><LF> 
 Field Number:  
  1) Latitude 
  2) N or S (North or South) 
  3) Longitude 
  4) E or W (East or West) 
  5) Universal Time Coordinated (UTC) 
  6) Status A - Data Valid, V - Data Invalid , P - Precise
  7) Checksum */
String lat;
String lon;
String lat_;
String lon_;
String UTC;
String sHH,sMM,sSS;
int hh;
int mm;
int ss;


int sensIN;
void loop() {
 
 while (Serial2.available()) {
    // get the new byte:
    char inChar = (char)Serial2.read();
    // add it to the inputString:
    
        // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '$') {
      dataString="";
      inputString="";
      dataPosOld=dataPos;
      dataPos=0;
      stringComplete = true;
    }  else {
      inputString += inChar;
      }  
    if (inChar == ',') {  
      //Serial.print(dataPos);
      //Serial.print(":");
     // Serial.println(dataString);

      
      if (dataPos==0){
        GPS_word=dataString;
      }else{
        if (GPS_word=="$GPGLL"){
        switch (dataPos)
        {
        case 1:
          /* code */
          lat=dataString;

          break;
        case 2:
         lat_=dataString;

          /* code */
          break;
           case 3:
           
lon=dataString;
          /* code */
          break;
           case 4:
           lon_=dataString;

          /* code */
          break;
           case 5:
           UTC=dataString;
          // String str=UTC.substring(0,2);
          sHH=UTC.substring(0,2);
          hh=sHH.toInt();
          sMM=UTC.substring(2,4);
          mm=sMM.toInt();
          sSS=UTC.substring(4,6);
          ss=sSS.toInt();
          /* code */
          break;
        //default:
         // break;
        }



    }


      }


       dataString="";
       dataPos++;
    }else{
      dataString+= inChar;
    }
    
    


  }

  /*
  GLL - Geographic Position - Latitude/Longitude 
           1       2 3        4 5         6 7 
       |       | |        | |         | | 
$--GLL,llll.ll,a,yyyyy.yy,a,hhmmss.ss,A*hh<CR><LF> 
 Field Number:  
  1) Latitude 
  2) N or S (North or South) 
  3) Longitude 
  4) E or W (East or West) 
  5) Universal Time Coordinated (UTC) 
  6) Status A - Data Valid, V - Data Invalid , P - Precise
  7) Checksum */

// $GPGGA,HHMMSS.ss,BBBB.BBBB,b,LLLLL.LLLL,l,Q,NN,D.D,H.H,h,G.G,g,A.A,RRRR*PP


  if (stringComplete){
     stringComplete = false;
     
    // Serial.println(inputString);
       if (GPS_word=="$GPGLL"){
          String Stamp=lat;
                  Stamp+=lat_;
                  Stamp+=";";
                  Stamp+=lon;
                  Stamp+=lon_;
                  Stamp+=";";
                   Stamp+=sHH;
                    Stamp+=":";
                     Stamp+=sMM;
                     Stamp+=":";
                     Stamp+=sSS;
  Serial.println(Stamp);
       }
  }    





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
 // LoRa.print("Repeated Message Nr: ");
  LoRa.print(LoRaData);
  LoRa.print(" | Recieved with RSSI: "); 
  LoRa.print(rssi_rx);
  LoRa.print("| Resent from Sender ");
  LoRa.print(myID);
  LoRa.print(" with Nr:");
  LoRa.print(counter); 
  
  Serial.println(LoRa.endPacket());
  counter++;
  if(counter>=10000){
    counter=0;
  }


   
  }







  
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    // say what you got:
   Serial.print(incomingByte, DEC);
     Serial.println(" on Serial 0");
  }

}