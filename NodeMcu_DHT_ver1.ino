// www.arduinesp.com 
//
// Plot DTH11 data on thingspeak.com using an ESP8266 
// April 11 2015
// Author: Jeroen Beemster
// Website: www.arduinesp.com
// changed for DHT22 (AM2302) by niq_ro (Nicu FLORICA)
// from http://nicuflorica.blogspot.ro 
 
#include <DHT.h>
#include <ESP8266WiFi.h>
 
// replace with your channel's thingspeak API key, 

String apiKey = "write_API_key";
const char* ssid = "SSID";
const char* password = "password";

const char* server = "api.thingspeak.com";
#define DHTalim 4 // power pin for DHT sensor
// note GPIO4 is D2 at NodeMCU - http://www.14core.com/wp-content/uploads/2015/06/Node-MCU-Pin-Out-Diagram1.png
// http://www.elecrow.com/download/AM2302.pdf

#define DHTPIN 5 // data pin for DHT sensor
// note GPIO5 is D1 at NodeMCU - http://www.14core.com/wp-content/uploads/2015/06/Node-MCU-Pin-Out-Diagram1.png
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22 
DHT dht(DHTPIN, DHTTYPE,11);
WiFiClient client;
   
 
void setup() {                
  Serial.begin(115200);
  delay(10);

  pinMode(DHTalim, OUTPUT); // pin for power the DHT sensor is outpin
  digitalWrite(DHTalim, HIGH); // pin for power the DHT sensor is outpin
  
  dht.begin();
  
  WiFi.begin(ssid, password);
 
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
   
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  
}
 
 
void loop() {
   
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if ((h == 0) and (t == 0)) {   // some DHT sensor has error after long time powered
  digitalWrite(DHTalim, LOW);
  delay(1000);
  digitalWrite(DHTalim, HIGH);
  delay(2000);
  // read again after reset sensor
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  }  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
 
  if (client.connect(server,80)) {  //   "184.106.153.149" or api.thingspeak.com
    String postStr = apiKey;
           postStr +="&field1=";
           postStr += String(t);
           postStr +="&field2=";
           postStr += String(h);
           postStr += "\r\n\r\n";
 
     client.print("POST /update HTTP/1.1\n"); 
     client.print("Host: api.thingspeak.com\n"); 
     client.print("Connection: close\n"); 
     client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n"); 
     client.print("Content-Type: application/x-www-form-urlencoded\n"); 
     client.print("Content-Length: "); 
     client.print(postStr.length()); 
     client.print("\n\n"); 
     client.print(postStr);
           
 
     Serial.print("Temperature: ");
     Serial.print(t);
     Serial.print(" degrees Celsius Humidity: "); 
     Serial.print(h);
     Serial.println("% send to Thingspeak");    
  }
  client.stop();
   
  Serial.println("Waiting...");    
  // thingspeak needs minimum 15 sec delay between updates
  delay(20000);  
}// www.arduinesp.com 
