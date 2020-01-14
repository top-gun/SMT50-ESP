# SMT50-ESP
Interface a SMT50-moisture sensor via an ADS1115 and transmit data via MQTT

Dokumentation Bodenfeuchtesensor

Sensor

Kernkomponenten sind der Wemos D1-Einplatinencomputer. Weil vom Trübner-Sensor zwei analoge Signale kommen, kommt noch ein ADS1115-Analog-Digital-Konverter dazu, der 4x Analogeingang hat und das digital an den Wemos weiterleitet. Normal wäre das schon alles was man braucht. Aber: Der Trübner-Sensor braucht 2,4mA Strom. Schließt man ihn direkt an die Versorgungsspannung, zieht er die 24x7 und nuckelt die Batterien binnen 3-4 Wochen leer. Deshalb bekommt der Sensor seine Stromversorgung über ein Relais. Das Relais wird über den Pin D7 vom Wemos kurz angeschaltet, nach dem Lesen der Sensordaten wird es wieder abgeschaltet. So sollte der Strom 2-3 Monate reichen.

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
