// Simple demo for feeding some random data to Pachube.
// 2011-07-08 <jc@wippler.nl> http://opensource.org/licenses/mit-license.php

// Handle returning code and reset ethernet module if needed
// 2013-10-22 hneiraf@gmail.com

// Modifing so that it works on my setup for www.thingspeak.com.
// Arduino pro-mini 5V/16MHz, ETH modul on SPI with CS on pin 10.
// Also added a few changes found on various forums. Do not know what the 
// res variable is for, tweaked it so it works faster for my application
// 2015-11-09 dani.lomajhenic@gmail.com
// niq_ro change the sketch for DHT11,DHT22 (AM2302) sensor
// 2016-02-04 nicu.florica@gmail.com - http://www.tehnic.go.ro
// thx to Cristi for sketch with BMP085 ;)

#include <EtherCard.h>

//#include <Wire.h>
//#include <Adafruit_BMP085.h>

// change these settings to match your own setup
//#define FEED "000"
#define APIKEY "Write API key" // put your key here
#define ethCSpin 8 // put your CS/SS pin here.

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
const char website[] PROGMEM = "api.thingspeak.com";
byte Ethernet::buffer[700];
uint32_t timer;
Stash stash;
byte session;

//timing variable
int res = 100; // was 0

//Adafruit_BMP085 bmp;

#include <DHT.h>
#define DHTPIN A1     // what pin we're connected DHT11/22
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22

DHT dht(DHTPIN, DHTTYPE);
//float te;
//int has;

float hit; // heat index, see http://www.eaglewings.ro/indicele-de-confort-termic/
// also see https://en.wikipedia.org/wiki/Heat_index

void setup () {
  Serial.begin(9600);
  Serial.println("\n[ThingSpeak example]");

  //Initialize Ethernet
  initialize_ethernet();

  /*
   if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }
*/

 dht.begin();  // DHT init 
  
}


void loop () { 
  //if correct answer is not received then re-initialize ethernet module
  if (res > 220){
    initialize_ethernet(); 
  }
  
  res = res + 1;
  
  ether.packetLoop(ether.packetReceive());
  
  //200 res = 10 seconds (50ms each res)
  if (res == 200) {
 /*  
  float temp = bmp.readTemperature();
  //t = t*100; // multiplicam cu 100 sa vedem si virgulele
  float p = bmp.readPressure();
  p = p/101.325;
  int pres = p*0.760;
*/

  float humidity = dht.readHumidity();
//float humidity = random(3000,5000);
//humidity = humidity / 100;
  
delay(500);  
  float temperature = dht.readTemperature();
//float temperature = random (2000,2500);
//temperature = temperature / 100;

delay(500);

hit = (temperature*1.8+32) - (0.55-0.0055*humidity)*((temperature*1.8+32)-58);
//$itu = ($t * 1.8 + 32) – (0.55 – 0.0055 * $u) * (($t * 1.8 + 32) – 58);
// http://www.eaglewings.ro/indicele-de-confort-termic/

    // generate two fake values as payload - by using a separate stash,
    // we can determine the size of the generated message ahead of time
    // field1=(Field 1 Data)&field2=(Field 2 Data)&field3=(Field 3 Data)&field4=(Field 4 Data)&field5=(Field 5 Data)&field6=(Field 6 Data)&field7=(Field 7 Data)&field8=(Field 8 Data)&lat=(Latitude in Decimal Degrees)&long=(Longitude in Decimal Degrees)&elevation=(Elevation in meters)&status=(140 Character Message)
    byte sd = stash.create();
    stash.print("field1=");
 //   stash.print(temp);
   stash.print(temperature);
    stash.print("&field2=");
//    stash.print(pres);
   stash.print(humidity);
    //stash.print("&field3=");
    //stash.print(msje);
   stash.print("&field3=");
   stash.print(hit);

    stash.save();

    //Display data to be sent
    Serial.print(temperature);
    Serial.println(" gr.C");
    Serial.print(humidity);
    Serial.println(" %RH");
    Serial.print("heat index = ");
    Serial.print(hit);
 if (hit <= 65) Serial.println(" ---> OK !");
 if ((hit > 65) and (hit < 79 )) Serial.println(" ---> CAUTION !");
 if (hit >= 80) Serial.println(" ---> DANGER !");
    Serial.println("-----------");

    
    // generate the header with payload - note that the stash size is used,
    // and that a "stash descriptor" is passed in as argument using "$H"
    Stash::prepare(PSTR("POST /update HTTP/1.0" "\r\n"
      "Host: $F" "\r\n"
      "Connection: close" "\r\n"
      "X-THINGSPEAKAPIKEY: $F" "\r\n"
      "Content-Type: application/x-www-form-urlencoded" "\r\n"
      "Content-Length: $D" "\r\n"
      "\r\n"
      "$H"),
    website, PSTR(APIKEY), stash.size(), sd);

    // send the packet - this also releases all stash buffers once done
    session = ether.tcpSend(); 

 // added from: http://jeelabs.net/boards/7/topics/2241
 int freeCount = stash.freeCount();
    if (freeCount <= 3) {   Stash::initMap(56); } 

  }  // aici se termina bucla de transmisie
  
   const char* reply = ether.tcpReply(session);
   
   if (reply != 0) {
     res = 0;
     Serial.println(F(" >>>REPLY recieved...."));
     Serial.println(reply);
   }
   delay(1500);
//   delay(150);

}  // end main loop



void initialize_ethernet(void){  
  for(;;){ // keep trying until you succeed 
    //Reinitialize ethernet module
    //pinMode(5, OUTPUT);  // do notknow what this is for, i ve got something elso on pin5
    //Serial.println("Reseting Ethernet...");
    //digitalWrite(5, LOW);
    //delay(1000);
    //digitalWrite(5, HIGH);
    //delay(500);

    if (ether.begin(sizeof Ethernet::buffer, mymac, ethCSpin) == 0){ 
      Serial.println( "Failed to access Ethernet controller");
      continue;
    }
    
    if (!ether.dhcpSetup()){
      Serial.println("DHCP failed");
      continue;
    }

    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);  
    ether.printIp("DNS: ", ether.dnsip);  

    if (!ether.dnsLookup(website))
      Serial.println("DNS failed");

    ether.printIp("SRV: ", ether.hisip);

    //reset init value
    res = 180;
    break;
  }
}
