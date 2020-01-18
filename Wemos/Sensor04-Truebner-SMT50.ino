#include <Wire.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_ADS1015.h>


// Update these with values suitable for your network.
const char* ssid = "xxxxwifiname";
const char* password = "secretpassword";
const char* mqtt_server = "192.168.178.37";

// DS-Intervall ist die Pause die der ESP im Deep Sleep bleibt. Produktion: 30 Minuten
const uint32_t DS_Intervall = 60*60*1000000;

// oder für Debuggingzwecke (dann nur 3s pro Durchgang)
//const uint32_t DS_Intervall = 3*60*1000000;

Adafruit_ADS1115 ads(0x4a);     /* Use this for the 16-bit version */

WiFiClient espClient;
PubSubClient client(espClient);

ADC_MODE(ADC_VCC);

void setup_wifi() {
// Adressen statisch konfigurieren. Passend für "Sensor04"
    IPAddress ip(192, 168, 178, 57);
    IPAddress gateway(192, 168, 178, 1);
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dns(192, 168, 178, 1);

    WiFi.config(ip, dns, gateway, subnet);
    
  // We start by connecting to a WiFi network
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
void setup()
{
  //Seriellen Port auf 9600 bps
  Serial.begin(74880);
  Serial.println("Truebner Feuchtigkeitssensor");

  // Feuchtigkeitssensor anschalten
  pinMode (D7, OUTPUT); // D7 ist Stromversorgung für Sensor
  digitalWrite (D7, HIGH);
  int16_t adc0, adc1;
  ads.begin();
  ads.setGain(GAIN_ONE);  // 2/3x gain +/- 6.144V  1 bit = 3mV (default) 

  // Temperatur und Feuchtigkeit abholen und seriell ausgeben    
  delay(400);
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

  
  // adc*0.125 ist die Spannung in Millivolt. *0.050/3 ergibt die volumetrische Feuchtigkeit
  float humd=float(int((adc0*0.125)*0.050/3*100+0.5))/100;
  // adc*0.125 ist die Spannung in Millivolt. (mV/1000 - 0.5)*100 ergibt die Temperatur
  
    float temp=adc1*0.125; // Sensor in millivolt
    temp=(temp/1000-0.5)*100;  // Millivolt in Temperatur
    temp=float( int(temp*10+0.5)) /10;
    //mqtt Server ansprechen
    client.setServer(mqtt_server, 1883);
    Serial.print("Attempting MQTT connection...");
    String clientId = "Sensor04";
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 30 minutes");
      ESP.deepSleep(DS_Intervall);  
      delay(100);
      // remember: After deep sleep, the program terminates
    }
  
    String msg="";
    char MsgFeuch[25];
    char MsgTemp[25];
    char Msgvcc[25];
    char Msgmillis[25];
    msg=humd;
    msg.toCharArray(MsgFeuch,25);
    client.publish("Sensoren/Sensor04/Feuchtigkeit",MsgFeuch);
    Serial.print("Feuchtigkeit an MQTT: ");
    Serial.println(MsgFeuch);
    msg=temp;
    msg.toCharArray(MsgTemp,25);
    client.publish("Sensoren/Sensor04/Temperatur",MsgTemp);
    Serial.print("Temperatur an MQTT: ");
    Serial.println(MsgTemp);
    msg= ESP.getVcc();
    msg.toCharArray(Msgvcc,25);
    client.publish("Sensoren/Sensor04/Spannung",Msgvcc);
    msg= millis();
    msg.toCharArray(Msgmillis,25);
    client.publish("Sensoren/Sensor04/Laufzeit",Msgmillis);

    // MQTT disconnect
    client.disconnect();

    Serial.print("Time after sending data: ");
    Serial.println(millis());

    
    Serial.println("300ms warten");
    delay(300);
    Serial.println("Jetzt Deep Sleep");
    ESP.deepSleep(DS_Intervall);  
    delay(100);
}

void loop()
{
}
