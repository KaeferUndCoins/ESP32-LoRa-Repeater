#include <EEPROM.h>
#include <WiFi.h>
#include <time.h>
#include <SPI.h>
#include <LoRa.h>
#include <TinyGPS++.h>
// GPS conected to
// RXD -> 17
// TXD -> 16
// GND -> GND
// VCC -> 5V
// PPS -> NC

const char *ssid = "VodafoneMobileWiFi-76E532";
const char *password = "0660946111";
const char *iphost = "raincloud.bplaced.net";

//String url = "/sentLora.php"
String url;
//define the pins used by the transceiver module
#define ss_pin 18
#define rst_pin 14
#define dio0_pin 2
//#define ADC_1 36
#define ledpin 25
// lora setting Variables

int loraSF = 7; //06-12 entspricht LIB default
int loraBW = 7; //00-09 entspricht LIB default

TinyGPSPlus gps;
double lat = 48.765045;
double lng = 9.147345;
int counter = 0;

time_t now = time(nullptr);

#define eeID 0x0
unsigned int myID;

#define eeExtCMD 0x1
unsigned int extCMD;

 long getBW(int bw){
    switch (bw) {

    case 0: return 7.8E3;

    case 1: return 10.4E3;

    case 2: return 15.6E3;

    case 3: return 20.8E3;

    case 4: return 31.25E3;

    case 5: return 41.7E3;

    case 6: return 62.5E3;

    case 7: return 125E3;

    case 8: return 250E3;

    case 9: return 500E3;

  }
 }



void setEEPROM()
{
  EEPROM.write(eeID, myID);
  EEPROM.write(eeExtCMD, extCMD);
  EEPROM.commit();
}

void getEEPROM()
{
  EEPROM.begin(512);
  // EEPROM.write(eeID,1);
  //EEPROM.commit();
  myID = EEPROM.read(eeID);
  extCMD = EEPROM.read(eeExtCMD);

  Serial.print("ID: ");
  Serial.println(myID);
  Serial.print("CMD: ");
  Serial.println(extCMD);
}

String gridSync()
{
  now = time(nullptr);
  Serial.println(ctime(&now));
  int myhh = localtime(&now)->tm_hour;
  int mymm = localtime(&now)->tm_min;
  int myss = localtime(&now)->tm_sec;
  int myYY = localtime(&now)->tm_year + 1900;
  int myMM = localtime(&now)->tm_mon + 1;
  int myDD = localtime(&now)->tm_mday;
  String str = "";
  str += ("{");
  str += ("\"MsgNr\":");
  str += (counter);
  str += (", \"TxID\":");
  str += (myID);
  str += (", \"SF\":");
  str += (loraSF);
  str += (", \"BW\":");
  str += (getBW(loraBW));
  if (gps.date.isValid())
  {
    str += (", \"Date\":\"");
    if (gps.date.day() < 10)
      str += ("0");
    str += (gps.date.day());
    str += (".");
    if (gps.date.month() < 10)
      str += ("0");
    str += (gps.date.month());
    str += (".");
    str += (gps.date.year());
    str += ("\"");
  }
  else
  {
    str += (", \"Date\":\"");
    if (myDD < 10)
      str += ("0");
    str += (myDD);
    str += (".");
    if (myMM < 10)
      str += ("0");
    str += (myMM);
    str += (".");
    str += (myYY);
    str += ("\"");
  }
  if (gps.time.isValid())
  {
    str += (", \"Time\":\"");
    if (gps.time.hour() < 10)
      str += ("0");
    str += (gps.time.hour());
    str += (":");
    if (gps.time.minute() < 10)
      str += ("0");
    str += (gps.time.minute());
    str += (":");
    if (gps.time.second() < 10)
      str += ("0");
    str += (gps.time.second());
    str += ("\"");
  }
  else
  {
    str += (", \"Time\":\"");
    if (myhh < 10)
      str += ("0");
    str += (myhh);
    str += (":");
    if (mymm < 10)
      str += ("0");
    str += (mymm);
    str += (":");
    if (myss < 10)
      str += ("0");
    str += (myss);
    str += ("\"");
  }
  if (gps.location.isValid())
  {
    str += (", \"Location\":");
    //{lat: -34, lng: 151}
    str += ("{\"lat\":");
    str += (gps.location.lat(), 6);
    str += (",");
    str += ("\"lng\":");
    str += (gps.location.lng(), 6);
    str += ("}");
  }
  str += (", \"Payload\":");
  str += ("{\"Beacon\":");
  str += (myID);
  
  str += (",\"Status\":\"Online\"}");

  //LoRa.print("\"");
  str += ("}");
  return str;
}

String postData(const char *thishost, String thisurl, String PostData)
{
  //ledON();
  Serial.print("connecting to ");
  Serial.println(thishost);
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(thishost, httpPort))
  {
    Serial.println("connection failed");
    return "connection failed";
  }
  Serial.print("Requesting URL: ");
  Serial.println(thisurl);
  // This will send the request to the server
  client.println("POST " + thisurl + " HTTP/1.1");
  client.println("Host: " + String(thishost)); //was macht das
  client.println("Cache-Control: no-cache");
  client.println("Content-Type: application/x-www-form-urlencoded"); //was macht das
  client.print("Content-Length: ");
  client.println(PostData.length());
  client.println();
  client.println(PostData);

  unsigned long timeout = millis();
  while (client.available() == 0)
  {
    if (millis() - timeout > 5000)
    {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return "timeout";
    }
  }
  //Serial.println("das habe ich gefunden :");
  String line = "";
  while (client.available())
  {
    line += client.readStringUntil('\r');
  }
  //Serial.print(line);
  //Serial.println("Was macht man jetzt damit");

  return line;

  //ledOFF();
}

String decodeJson(String inputStr, String searchStr, String type)
{

  int del_Anf;
  int del_End;
  String str;
  // #############
  // keyword suchen
  del_Anf = inputStr.indexOf(searchStr);
  // Serial.println(del_Anf);
  // trenner suchen
  del_Anf = inputStr.indexOf(":", del_Anf) + 1;
  //  Serial.println(del_Anf);

  del_End = inputStr.indexOf(",", del_Anf);
 
  str = inputStr.substring(del_Anf, del_End);
  
  int del_test=str.indexOf("}");
  if (del_test != -1){
    str=str.substring(0,del_test);
  }
  // Serial.println(str);

  if (type == "String")
  {
    del_Anf = str.indexOf("\"") + 1;
    // Serial.println(timeAnf);
    del_End = str.lastIndexOf("\"");
    //Serial.println(timeEnd);
    str = str.substring(del_Anf, del_End);
  }

  // Serial.println(str);
  return str;
}

void getCMD(String inStr)
{ 
  int BWin;
  int SFin;
  String valIn;
 // Serial.println(inStr);

  String searchStr = "SF";
  //Serial.print(searchStr);
  //Serial.print(": ");
  valIn=decodeJson(inStr, searchStr, "int");
  //Serial.println(valIn);
  SFin=valIn.toInt();
 //Serial.println(loraSF);
 searchStr = "BW";
//  Serial.print(searchStr);
//  Serial.print(": ");
  valIn=decodeJson(inStr, searchStr, "int");
//  Serial.println(valIn);
  BWin=valIn.toInt();
 if (SFin!=-1){
 Serial.print("LoRa Modul set to SF: ");
 Serial.println(SFin); 
 loraSF= SFin;
 LoRa.setSpreadingFactor(loraSF);
 }else{
   Serial.println("LoRa Modul set to SF: KEEP");
 }
 if (BWin!=-1){
 long newBW=getBW(BWin);
 Serial.print("LoRa Modul set to BW: ");
 Serial.println(newBW);
 loraBW=BWin;
 LoRa.setSignalBandwidth(newBW);
  }else{
   Serial.println("LoRa Modul set to BW: KEEP");
 }

  //Serial.println(inStr);
}
void setup()
{
  Serial.begin(9600);
  Serial2.begin(9600);
  /* * /
   EEPROM.begin(512);
   myID=6;
   setEEPROM();
  /* */

  getEEPROM();

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
  //a LoRa.setSyncWord(0x8);
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  /* */
  url = "/sentLora.php?id=";
  url += myID;
  WiFi.begin(ssid, password);
  //WiFi.begin(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  /* */

  /* */
  delay(1000);
  int timezone = 2;
  int dst = 0;
  configTime(timezone * 3600, dst * 0, "pool.ntp.org", "time.nist.gov");
  //configTime(timezone * 3600, dst * 0, "pool.ntp.org");
  Serial.println("\nWaiting for time");
  while (!time(nullptr))
  {
    Serial.print(".");
    delay(1000);
  }
  now = time(nullptr);
  Serial.println(ctime(&now));

  now = time(nullptr);
  Serial.println(ctime(&now));

  Serial.print("Sending Gridsync");
  // Serial.println(gridSync());
  //LoRa.dumpRegisters(Serial);
  getCMD(postData(iphost, url, gridSync()));
  
 // LoRa.dumpRegisters(Serial);

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
int lastMsg;
int interval = 60000;
void loop()
{

  if (millis() - lastMsg >= interval)
  {
    lastMsg = millis();
    Serial.print("Sending Hartbeat: ");
    Serial.println(counter);
    //Send LoRa packet to receiver
    String hartbeat = "";

    hartbeat += ("{");
    hartbeat += ("\"MsgNr\":");
    hartbeat += (counter);
    hartbeat += (", \"TxID\":");
    hartbeat += (myID);
    hartbeat += (", \"SF\":");
    hartbeat += (loraSF);
    hartbeat += (", \"BW\":");
    hartbeat += (getBW(loraBW));
    if (gps.date.isValid())
    {
      hartbeat += (", \"Date\":\"");
      if (gps.date.day() < 10)
        hartbeat += ("0");
      hartbeat += (gps.date.day());
      hartbeat += (".");
      if (gps.date.month() < 10)
        hartbeat += ("0");
      hartbeat += (gps.date.month());
      hartbeat += (".");
      hartbeat += (gps.date.year());
      hartbeat += ("\"");
    }
    if (gps.time.isValid())
    {
      hartbeat += (", \"Time\":\"");
      if (gps.time.hour() < 10)
        hartbeat += ("0");
      hartbeat += (gps.time.hour());
      hartbeat += (":");
      if (gps.time.minute() < 10)
        hartbeat += ("0");
      hartbeat += (gps.time.minute());
      hartbeat += (":");
      if (gps.time.second() < 10)
        hartbeat += ("0");
      hartbeat += (gps.time.second());
      hartbeat += ("\"");
    }
    if (gps.location.isValid())
    {
      hartbeat += (", \"Location\":");
      //{lat: -34, lng: 151}
      hartbeat += ("{\"lat\":");
      hartbeat += (gps.location.lat(), 6);
      hartbeat += (",");
      hartbeat += ("\"lng\":");
      hartbeat += (gps.location.lng(), 6);
      hartbeat += ("}");
    }
    hartbeat += (", \"Payload\":");
    hartbeat += ("\"Tragt einen Aluhut\"");
    //  hartbeat+=("\"");
    hartbeat += ("}");

    LoRa.beginPacket();
    LoRa.print(hartbeat);
    LoRa.endPacket();
    Serial.println(hartbeat);
    getCMD(postData(iphost, url, hartbeat));

    counter++;
    if (counter >= 10000)
    {
      counter = 0;
    }
  }

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
  // read packet
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    String LoRaData = "";
    int RSSI = LoRa.packetRssi();
    // received a packet
    Serial.print("Received packet '");
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(RSSI);

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

    String json_Msg = "{";
    json_Msg += ("\"MsgNr\":");
    json_Msg += (counter);
    json_Msg += (", \"RxID\":");
    json_Msg += (myID);
    json_Msg += (", \"SF\":");
    json_Msg += (loraSF);
    json_Msg += (", \"BW\":");
    json_Msg += (getBW(loraBW));
    json_Msg += (", \"RSSI\":");
    json_Msg += (rssi_rx);
    if (gps.date.isValid())
    {
      json_Msg += (", \"Date\":\"");
      if (gps.date.day() < 10)
        json_Msg += ("0");
      json_Msg += (gps.date.day());
      json_Msg += (".");
      if (gps.date.month() < 10)
        json_Msg += ("0");
      json_Msg += (gps.date.month());
      json_Msg += (".");
      json_Msg += (gps.date.year());
      json_Msg += ("\"");
    }
    if (gps.time.isValid())
    {
      json_Msg += (", \"Time\":\"");
      if (gps.time.hour() < 10)
        json_Msg += ("0");
      json_Msg += (gps.time.hour());
      json_Msg += (":");
      if (gps.time.minute() < 10)
        json_Msg += ("0");
      json_Msg += (gps.time.minute());
      json_Msg += (":");
      if (gps.time.second() < 10)
        json_Msg += ("0");
      json_Msg += (gps.time.second());
      json_Msg += ("\"");
    }
    if (gps.location.isValid())
    {
      lat = gps.location.lat();
      lng = gps.location.lng();
    }
    json_Msg += (", \"Location\":");
    json_Msg += ("{\"lat\":");
    json_Msg += String(lat, 6);
    json_Msg += (",");
    json_Msg += ("\"lng\":");
    json_Msg += String(lng, 6);
    json_Msg += ("}");

    json_Msg += (", \"Payload\":");
    json_Msg += (LoRaData);
    //LoRa.print("\"");
    json_Msg += ("}");
    ++counter;
    Serial.print(json_Msg);
    LoRa.beginPacket();
    LoRa.print(json_Msg);
    LoRa.endPacket();
    Serial.println(postData(iphost, url, json_Msg));
  }
  /*
  int packetSize = LoRa.parsePacket();
  if (packetSize)ä#
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
  */

  if (Serial.available() > 0)
  {
    // read the incoming byte:
    char incomingByte = Serial.read();
    // say what you got:
    Serial.print(incomingByte, DEC);
    Serial.println(" on Serial 0");
  }
}