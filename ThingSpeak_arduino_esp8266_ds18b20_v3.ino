
// base info: http://www.instructables.com/id/ESP8266-Wifi-Temperature-Logger/?ALLSTEPS
//  this version is adapted by niq_ro (Nicu FLORICA)
// from http://arduinotehniq.com ( http://www.tehnic.go.ro & http://nicuflorica.blogspot.ro )
//https://github.com/tehniq3/ThingSpeak

#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


//*-- IoT Information
#define SSID "internetnetwork"
#define PASS "password"

#include <SoftwareSerial.h>
#include <stdlib.h>

// LED 
int ledPin = 13;

// replace with your channel's thingspeak API key
String apiKey = "apikey";

// connect 10 to TX of Serial USB
// connect 11 to RX of serial USB
//SoftwareSerial ser(10, 11); // RX, TX
SoftwareSerial ser(11, 10); // RX, TX

byte contorizare = 0;

// this runs once
void setup() {                
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);    

  // enable debug serial
  Serial.begin(19200); 
  Serial.println("-----------------------");
  // enable software serial
  ser.begin(19200);

sensors.begin();  //for dallas temp sensor
 // 9 bit resolution by default 
  // Note the programmer is responsible for the right delay
  // we could do something usefull here instead of the delay
  int resolution = 11;
  sensors.setResolution(resolution);
  delay(750/ (1 << (12-resolution)));
//  checkSensors();   //check all available sensors and save values to RAM
                       
   connectWiFi();
}


// the loop 
void loop() {
  
  // blink LED on board
  digitalWrite(ledPin, HIGH);   
  delay(200);               
  digitalWrite(ledPin, LOW);

sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
//  tempC = DallasTemperature::toFahrenheit(tempC);
  char buffer[10];
  String strTemp = dtostrf(tempC, 4, 1, buffer);
   
Serial.println(strTemp);
  
  // TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "184.106.153.149"; // api.thingspeak.com
  cmd += "\",80";
  ser.println(cmd);
   
  if(ser.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
  }
  
  // prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(strTemp);
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  ser.println(cmd);

  if(ser.find(">")){
    ser.print(getStr);
  }
  else{
    ser.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSE");
  }
    
  // thingspeak needs 15 sec delay between updates
//  delay(16000);  // 16 seconnds interval (between send datas)
    delay(300000);  // 5 minutes (300 seconnds) interval (between send datas)
//    delay(900000);  // 15 minutes (900 seconnds) interval (between send datas)

contorizare = contorizare + 1;

if (contorizare > 20)
{
 connectWiFi(); 
 contorizare = 0;
}
}

boolean connectWiFi()
{
  
   // reset ESP8266
   Serial.println("AT+RST");
   ser.println("AT+RST");

  for (byte clipire = 0; clipire < 20; clipire++) 
  {
  digitalWrite(ledPin, HIGH);   
  delay(100);               
  digitalWrite(ledPin, LOW);
  delay(100); 
  digitalWrite(ledPin, HIGH);   
  delay(100);               
  digitalWrite(ledPin, LOW);
  delay(700); 
  }

 // Serial.println("AT+CWMODE=1");//WiFi STA mode - if '3' it is both client and AP
  String cmd0="AT+CWMODE=1"; // Join accespoint
  sendDebug(cmd0);
  delay(2000);
  
  //Connect to Router with AT+CWJAP="SSID","Password";
  // Check if connected with AT+CWJAP?
  String cmd="AT+CWJAP=\""; // Join accespoint
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"";
  sendDebug(cmd);
  delay(5000);
  if(Serial.find("OK"))
  {
     ser.println("RECEIVED: OK");
    return true;
  }
  else
  {
     ser.println("RECEIVED: Error");
    return false;
  }
}

void sendDebug(String cmd)
{
  ser.print("SEND");
  ser.println(cmd);
  Serial.println(cmd);
}
