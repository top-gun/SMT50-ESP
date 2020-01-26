#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_ADS1015.h>

// Update these with values suitable for your network.
const char* ssid = "xxxxxx";
const char* password = "yyyyyyyy";
const char* mqtt_server = "192.168.178.37";

// DS-Intervall is one minute in Mikroseconds, needed for the calculation of the Deep Sleep between measurements
const uint32_t DS_Intervall = 60*1000000;
// Minuten is the Deep Sleep-time between measurements
uint32_t Minuten = 30;

// The ESP8266 RTC memory is arranged into blocks of 4 bytes. The access methods read and write 4 bytes at a time,
// so the RTC data structure should be padded to a 4-byte multiple.
struct {
  uint32_t crc32;   // 4 bytes
  uint8_t channel;  // 1 byte,   5 in total
  uint8_t bssid[6]; // 6 bytes, 11 in total
  uint8_t padding;  // 1 byte,  12 in total
} rtcData;

// or use this for debugging purposes: 3 minutes
//const uint32_t DS_Intervall = 3*60*1000000;

//  define object for the handling of the ADC-chip
Adafruit_ADS1115 ads(0x4a);     // Use this for the 16-bit version

// define object for the WiFi-communication
WiFiClient espClient;

// define object for MQTT-communication to the server
PubSubClient client(espClient);

// configure the internal ADC to monitor the 3.3V-powersupport
ADC_MODE(ADC_VCC);

void setup_wifi() {

// Use Wifi.config to manually configure the network. 
// static address configuration. You can leave these out and use DHCP by using WiFi.begin() instead of WiFi.config
  IPAddress ip(192, 168, 178, 57);
  IPAddress gateway(192, 168, 178, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(192, 168, 178, 1);
  WiFi.config(ip, dns, gateway, subnet);

// Try to read WiFi settings from RTC memory
  bool rtcValid = false;
  if( ESP.rtcUserMemoryRead( 0, (uint32_t*)&rtcData, sizeof( rtcData ) ) ) {
  // Calculate the CRC of what we just read from RTC memory, but skip the first 4 bytes as that's the checksum itself.
    uint32_t crc = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
    if( crc == rtcData.crc32 ) {
      rtcValid = true;
    }
  }


// Bring up the WiFi connection
  WiFi.forceSleepWake();
  delay( 1 );
  WiFi.mode( WIFI_STA );

// We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  if( rtcValid ) {
    // The RTC data was good, make a quick connection
    Serial.println("connecting with data from rtc memory");
    WiFi.begin(ssid,password, rtcData.channel, rtcData.bssid, true );
  }
  else {
    // The RTC data was not valid, so make a regular connection
    Serial.println("connecting fresh, just SSID and password");
    WiFi.begin(ssid,password);
  }


  int retries = 0;
  int wifiStatus = WiFi.status();
  while( wifiStatus != WL_CONNECTED ) {
    retries++;
    if( retries == 100 ) {
      // Quick connect is not working, reset WiFi and try regular connection
      Serial.println("Changing strategy, attemting fresh connection");
      WiFi.disconnect();
      delay( 10 );
      WiFi.forceSleepBegin();
      delay( 10 );
      WiFi.forceSleepWake();
      delay( 10 );
      WiFi.begin(ssid,password);
    }
    if( retries == 600 ) {
      // Giving up after 30 seconds and going back to sleep
      Serial.println("Giving up after 30 seconds without success");
      WiFi.disconnect( true );
      delay( 1 );
      WiFi.mode( WIFI_OFF );
      ESP.deepSleep( Minuten * DS_Intervall, WAKE_RF_DISABLED );
      return; // Not expecting this to be called, the previous call will never return.
    }
    delay( 50 );
    wifiStatus = WiFi.status();
  }
//  Serial.println("");
//  Serial.println("WiFi connected");
//  Serial.println("IP address: ");
//  Serial.println(WiFi.localIP());
//  uint8_t macAddr[6];
//  WiFi.macAddress(macAddr);
//  Serial.printf("mac address: %02x:%02x:%02x:%02x:%02x:%02x:\n",macAddr[0],macAddr[1],macAddr[2],macAddr[3],macAddr[4],macAddr[5]);
  // Write current connection info back to RTC
  rtcData.channel = WiFi.channel();
  memcpy( rtcData.bssid, WiFi.BSSID(), 6 ); // Copy 6 bytes of BSSID (AP's MAC address)
  rtcData.crc32 = calculateCRC32( ((uint8_t*)&rtcData) + 4, sizeof( rtcData ) - 4 );
  ESP.rtcUserMemoryWrite( 0, (uint32_t*)&rtcData, sizeof( rtcData ) );

} // end of setup_wifi

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  String payloadname = String((char*)topic);
  if (payloadname == "Sensoren/Sensor03/Intervall") {
    payload[length] = '\0';
    String s = String((char*)payload);
    Minuten = s.toInt();
    if (Minuten == 0) {
      Minuten = 30; 
    }
    if (Minuten > 60) {
      Minuten = 60; 
    }
    Serial.println(Minuten);
  }

} // end of callback

uint32_t calculateCRC32( const uint8_t *data, size_t length ) {
  uint32_t crc = 0xffffffff;
  while( length-- ) {
    uint8_t c = *data++;
    for( uint32_t i = 0x80; i > 0; i >>= 1 ) {
      bool bit = crc & 0x80000000;
      if( c & i ) {
        bit = !bit;
      }

      crc <<= 1;
      if( bit ) {
        crc ^= 0x04c11db7;
      }
    }
  }

  return crc;
}

void setup()
{
// turn off wifi to conserve power  
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay( 1 );

// Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings in the flash memory.
  WiFi.persistent( false );
  delay(1);  
  Serial.begin(74880);
  Serial.println("Truebner SMT50 Soil moisture sensor");

  // Power up moisture sensor
  pinMode (D7, OUTPUT); // D7 switches the relay
  digitalWrite (D7, HIGH);
  int16_t adc0, adc1;
  ads.begin();
  ads.setGain(GAIN_ONE);  // 1 bit = 0.125mV in this mode

  // Temperatur und Feuchtigkeit abholen und seriell ausgeben    
  delay(400); // Truebner requires 300ms before the sensor data is stable. Using 400mV to allow for tolerances.
  adc0 = ads.readADC_SingleEnded(0);
  adc1 = ads.readADC_SingleEnded(1);
    
  Serial.print("ADC0: ");
  Serial.println(adc0);
  Serial.print("ADC1: ");
  Serial.println(adc1);

  Serial.print("Time after reading sensor: ");
  Serial.println(millis());

  // turn off moisture sensor
  digitalWrite (D7, LOW);

  // turn on wifi
  setup_wifi();

  Serial.print("Time after connecting wifi: ");
  Serial.println(millis());

  // convert sensor data and send it to server 
  // adc*0.125 is the signal voltage in millivolt. *0.050/3 is the conversion formula to volumetric humidity

  float humd=float(int((adc0*0.125)*0.050/3*100+0.5))/100;
 
  // adc*0.125 is the signal voltage in millivolt. (mV/1000 - 0.5)*100 is the formula to calculate the temperature
  
  float temp=adc1*0.125; // Sensor in millivolt
  temp=(temp/1000-0.5)*100;  // Millivolt in Temperatur
  temp=float( int(temp*10+0.5)) /10;

  //mqtt Server ansprechen
  
  client.setServer(mqtt_server, 1883);
  Serial.print("Attempting MQTT connection...");
  String clientId = "Sensor03";
  
  // Attempt to connect
  if (client.connect(clientId.c_str()))
  {
    Serial.println("connected");
  } else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 minutes");
    ESP.deepSleep(5 * DS_Intervall);  
    delay(100);
  // remember: After deep sleep, the program terminates
  }
// send data to the server via MQTT  
  String msg="";
  char MsgFeuch[25];
  char MsgTemp[25];
  char Msgvcc[25];
  char Msgmillis[25];

  msg=humd;
  msg.toCharArray(MsgFeuch,25);
  client.publish("Sensoren/Sensor03/Feuchtigkeit",MsgFeuch);
  Serial.print("Feuchtigkeit an MQTT: ");
  Serial.println(MsgFeuch);
  
  msg=temp;
  msg.toCharArray(MsgTemp,25);
  client.publish("Sensoren/Sensor03/Temperatur",MsgTemp);
  Serial.print("Temperatur an MQTT: ");
  Serial.println(MsgTemp);
  
  msg= ESP.getVcc();
  msg.toCharArray(Msgvcc,25);
  client.publish("Sensoren/Sensor03/Spannung",Msgvcc);
  
  msg= millis();
  msg.toCharArray(Msgmillis,25);
  client.publish("Sensoren/Sensor03/Laufzeit",Msgmillis);

// set callback for MQTT
  client.setCallback(callback); 
  client.subscribe("Sensoren/Sensor03/Intervall",1);

  Serial.print("Time after sending data: ");
  Serial.println(millis());
    
  Serial.println("300ms pause to make sure the server has responded to MQTT subscription");
  delay(300);
    
//hope for MQTT-callback
  client.loop();

// MQTT disconnect
  client.disconnect();  

// no wait because we have 100ms further down
  Serial.print(".");
  Serial.print("Jetzt Deep Sleep ");
  Serial.print(Minuten);
  WiFi.disconnect( true );
  delay( 10 );
  ESP.deepSleep(Minuten * DS_Intervall, WAKE_RF_DISABLED);  
  delay(100);
}

void loop()
{
}
