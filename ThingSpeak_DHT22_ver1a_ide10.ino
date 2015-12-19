/*
https://github.com/iobridge/ThingSpeak-Arduino-Examples/edit/master/Ethernet/Arduino_to_ThingSpeak.ino
http://community.thingspeak.com/documentation/api/
http://www.instructables.com/id/Measure-Temperature-with-Arduino-Ethernet-DS18B20-/?ALLSTEPS
 Arduino --> ThingSpeak Channel via Ethernet
 
 sketch adapted by niq_ro
 
 The ThingSpeak Client sketch is designed for the Arduino and Ethernet.
 This sketch updates a channel feed with an analog input reading via the
 ThingSpeak API (https://thingspeak.com/docs)
 using HTTP POST. The Arduino uses DHCP and DNS for a simpler network setup.
 The sketch also includes a Watchdog / Reset function to make sure the
 Arduino stays connected and/or regains connectivity after a network outage.
 Use the Serial Monitor on the Arduino IDE to see verbose network feedback
 and ThingSpeak connectivity status.
 
 Getting Started with ThingSpeak:
 
   * Sign Up for New User Account - https://thingspeak.com/users/new
   * Create a new Channel by selecting Channels and then Create New Channel
   * Enter the Write API Key in this sketch under "ThingSpeak Settings"
 
 Arduino Requirements:
 
   * Arduino with Ethernet Shield or Arduino Ethernet
   * work better with Arduino 1.0+ IDE
   
  Network Requirements:
   * Ethernet port on Router    
   * DHCP enabled on Router
   * Unique MAC Address for Arduino
 
 Created: October 17, 2011 by Hans Scharler (http://www.nothans.com)
 
 Additional Credits:
 Example sketches from Arduino team, Ethernet by Adrian McEwen
 
 Changed sketch by niq_ro (Nicu FLORICA) - http://www.tehnic.go.ro
 
*/

#include <SPI.h>
#include <Ethernet.h>

// Local Network Settings
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };   //physical mac address
byte ip[] = { 192, 168, 2, 50 };                      // ip in lan (that's what you need to use in your browser. ("192.168.1.178")
byte gateway[] = { 192, 168, 2, 1 };                   // internet access via router
byte subnet[] = { 255, 255, 255, 0 };                  //subnet mask
//EthernetServer server(80);                             //server port     
// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "Mwrite_API_key";
const int updateThingSpeakInterval = 25 * 1000;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

// Variable Setup
long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;

// Initialize Arduino Ethernet Client
EthernetClient client;


#include <DHT.h>
#define DHTPIN A1     // what pin we're connected DHT11/22
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22

DHT dht(DHTPIN, DHTTYPE);
float te, te0;
float has, has0;
int te1, te2;
int has1, has2;

void setup()
{
  // Start Serial for debugging on the Serial Monitor
  Serial.begin(9600);

 // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip, gateway, subnet);
//  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

dht.begin();  // DHT init 
  
  // Start Ethernet on Arduino
  startEthernet();
}

void loop()
{
  // Read value from Analog Input Pin 0
//  String analogValue0 = String(analogRead(A0), DEC);

te = dht.readTemperature();  // read temperature
te0 = 10*te;                 // multiplied with 10
te1 = te0/10;                // integer value
te2 = te0 - te1*10;          // value after point

has = dht.readHumidity();
has0 = 10*has;
has1 = has0/10;                // integer value
has2 = has0 - has1*10;          // value after point

//String analogValue0 = String(te, DEC);
//String analogValue1 = String(has, DEC);

String analogValue0 = String(te1, DEC);
analogValue0 += ".";
analogValue0 += String(te2, DEC);

String analogValue1 = String(has1, DEC);
analogValue1 += ".";
analogValue1 += String(has2, DEC);


  //String analogValue0 = String(24.5, DEC);
  //String analogValue1 = String(37, DEC);
  // Print Update Response to Serial Monitor
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }

  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected)
  {
    Serial.println("...disconnected");
    Serial.println();
    
    client.stop();
  }
  
  // Update ThingSpeak
  if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {
//    updateThingSpeak("field1="+analogValue0);
     updateThingSpeak("field1="+analogValue0+"&field2="+analogValue1);
     
  }
  
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) {startEthernet();}
  
  lastConnected = client.connected();
}

void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakAddress, 80))
  {         
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+ writeAPIKey +"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
      Serial.println();
    }
    
  }
  else
  {
    failedCounter++;
    
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    Serial.println();
    
    lastConnectionTime = millis(); 
  }
}

void startEthernet()
{
  
  client.stop();

  Serial.println("Connecting Arduino to network...");
  Serial.println();  

  delay(1000);
  
  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  }
  else
  {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
  }
  
  delay(1000);
}
