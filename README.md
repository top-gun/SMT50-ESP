# SMT50-ESP
Interface a SMT50-moisture sensor via an ADS1115 and transmit data via MQTT

Usecase: Control lawn irrigation with a soil moisture sensor. The sensor (Truebner SMT50) is connected to a Wemos D1 (ESP8266). Data about moisture and temperature is transmitted via WiFi and MQTT-protocol to a MQTT-broker inhouse. The project uses ioBroker as a MQTT-broker, and there is a second script on the ioBroker-server that remotely controls an irrigation clock by means of a Wifi-relay.

The core component is a Wemos D1-microcomputer. The Truebner-sensor offers two datums as analogue signals (0-3V). Since the Wemos has only one analogue input, I added an ADC-chip ADS1115. The ADS1115 can sample up to four analogue signals and connects to the Wemos via the I2C-protocol.

The project is supposed to run from 3 AA-batteries, so low consumption is an important requirement. The Truebner sensor has a permanent power consumption of 2.7 mA, this would drain the battery unacceptably fast. The solution was to switch the sensor with a cheap 5V-relay that is known to operate well on 3.3V. The sensor is powered for about 0.4s during each cycle. The manufacturer recommends 0.3s for the sensor to become stable, 0.4s compensates for the mechanical latency of the relay.

The Wemos D1 will supply one reading of both moisture and temperature, then switch to Deep Sleep for approcimately 60 minutes. This cycle uses around 10mAh per day, so the sensor is expected to last the whole year with just one exchange of batteries during the growing season. My setup allows for an easy upgrade to bigger batteries or even Li-Io cells plus solar charging.

For further documentation look at the "Documentation"-folder:

documentation/architecture is a diagram of the information-flow
documentation/parts-list-Teileliste is a text document with all used parts and links to potential sources
documentation/truebner-smt50.png shows sampled data as a flot-diagram (ioBroker)
documentation/Bodensensor-offen.jpg shows the assembled sensor device
documentation/Sensor-Schaltbild.png shows the wiring in the sender case.
