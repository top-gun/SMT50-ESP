#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_ADS1015.h>

// Update these with values suitable for your network.
const char* ssid = "xxxxwifiname";
const char* password = "secretpassword";
const char* mqtt_server = "192.168.178.37";

// DS-Intervall is one minute in Mikroseconds, needed for the calculation of the Deep Sleep between measurements
const uint32_t DS_Intervall = 60*1000000;
// Minuten is the Deep Sleep-time between measurements
uint32_t Minuten = 30;

// or use this for debugging purposes: 3 minutes
//const uint32_t DS_Intervall = 3*60*1000000;

//  define object for the handling of the ADC-chip
Adafruit_ADS1115 ads(0x48);     /* Use this for the 16-bit version */

// define object for the WiFi-communication
WiFiClient espClient;

// define object for MQTT-communication to the server
PubSubClient client(espClient);

// configure the internal ADC to monitor the 3.3V-powersupport
ADC_MODE(ADC_VCC);

void setup_wifi() {
// static address configuration. You can leave these out and use DHCP by using WiFi.begin() instead of WiFi.config
//  IPAddress ip(192, 168, 178, 57);
//  IPAddress gateway(192, 168, 178, 1);
//  IPAddress subnet(255, 255, 255, 0);
//  IPAddress dns(192, 168, 178, 1);

// Use Wifi.config to manually configure the network. Use Wifi.begin() for DHCP-controlled network.
//    WiFi.config(ip, dns, gateway, subnet);
  WiFi.begin();
    
// We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
} // end of setup_wifi

void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  String payloadname = String((char*)topic);
  if (payloadname == "Sensoren/Sensor03/Intervall") {
    payload[length] = '\0';
    String s = String((char*)payload);
    Minuten = s.toInt();
    if (Minuten == 0) {
      Minuten = 30; 
    }
    Serial.println(Minuten);
  }

} // end of callback


void setup()
{
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
  delay(100);
  ESP.deepSleep(Minuten * DS_Intervall);  
  delay(100);
}

void loop()
{
}
