# SMT50-ESP
Interface a SMT50-moisture sensor via an ADS1115 and transmit data via MQTT

Usecase: Measure soil moisture and soil temperatur with a Truebner SMT50-sensor, and wirelessly transfer the data to an MQTT broker. The project uses ioBroker as a MQTT-broker, and there is a second script on the ioBroker-server that remotely controls an irrigation clock by means of a Wifi-relay.

The core component is a Wemos D1-microcomputer. The Truebner-sensor offers two datums as analogue signals (0-3V). Since the Wemos has only one analogue input, I added an ADC-chip ADS1115. The ADS1115 can sample up to four analogue signals and connects to the Wemos via the I2C-protocol.

The project is supposed to run from 3 AA-batteries, so low consumption is an important requirement. The Truebner sensor has a permanent power consumption of 2.7 mA, this would drain the battery unacceptably fast. The solution was to switch the sensor with a cheap 5V-relay that is known to operate well on 3.3V. The sensor is powered for about 0.4s during each cycle. The manufacturer recommends 0.3s for the sensor to become stable, 0.4s compensates for the mechanical latency of the relay.

The Wemos D1 will supply one reading of both moisture and temperature, then switch to Deep Sleep for approcimately 60 minutes. This cycle uses around 10mAh per day, so the sensor is expected to last the whole year with just one exchange of batteries during the growing season. My setup allows for an easy upgrade to bigger batteries or even Li-Io cells plus solar charging.

Teileliste:

1.) Sensor "Trübner SMT50": 69 Euro - https://dvs-beregnung.de/kapazitiver-bodenfeuchte-sensor-inkl-temperaturmessung-truebner-smt50-arduino

2.) Wemos D1 Mini (Prozessor: ESP8266): 3 Euro - https://www.banggood.com/Geekcreit-D1-Mini-V3_0_0-WIFI-Internet-Of-Things-Development-Board-Based-ESP8266-4MB-p-1264245.html?rmmds=search&cur_warehouse=CN

3.) AD-Converter ADS1115: 7 Euro - https://www.banggood.com/3Pcs-CJMCU-ADS1115-16Bit-ADC-Development-Board-Module-p-1051693.html?rmmds=search&cur_warehouse=CN

4.) 5V-Relais (geht auch mit 3,3V) - 6 Euro (5er-Pack): https://www.banggood.com/5Pcs-5V-Relay-5-12V-TTL-Signal-1-Channel-Module-High-Level-Expansion-Board-For-Arduino-p-1178210.html?rmmds=search&cur_warehouse=CN​​

5.) Batteriefach 3x AA: 1,50 Euro - https://www.banggood.com/Transparent-Battery-Box-Holder-Fully-Sealed-with-Switch-for-3-x-AA-Batteries-p-1474109.html?rmmds=search&cur_warehouse=CN​

6.)​​​ Gehäuse ABS spritzwasserdicht: 5,50 Euro - https://www.banggood.com/158-x-90-x-46mm-DIY-Plastic-Waterproof-Housing-Electronic-Junction-Case-Power-Box-Instrument-Case-p-1261302.html?rmmds=myorder&cur_warehouse=CN

7.) Kabelverschraubung M12 (10 Stück): 6,50 Euro - https://smile.amazon.de/gp/product/B078YRNQL5/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1​

8.) Montagematerial: Platinen, Schraubklemmen, Chipfassungen: 9,99 Euro - https://smile.amazon.de/gp/product/B0728HZHTR/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1​


Zur Schaltung des Wemos und zur Programmerstellung hilft der Artikel:

http://www.esp8266learning.com/ads1115-analog-to-digital-converter-and-esp8266.php​

Relais:

Sonoff 4CH Pro R2 mit Tasmota-Software. MQTT an den ioBroker konfiguriert mit Port 1885 (spezieller Port, da Sonoff einen eigenen MQTT-Server mitbringt)

Alternativ kann ein Shelly 1-Relais eingesetzt werden. ioBroker bietet einen Adapter, um Shelly ohne Umbauten oder Umflashen zu steuern.
