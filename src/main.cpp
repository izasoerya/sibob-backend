#include <Arduino.h>
#include <WiFi.h>
#include <ElegantOTA.h>
#include "models.h"
#include "connection_wifi.h"

#include "sensor_reading/dht_reading.h"
#include "sensor_reading/ds18_reading.h"
#include "sensor_reading/hx711_reading.h"
#include "sensor_reading/ph_reading.h"
#include "sensor_reading/soil_hum_reading.h"
#include "control_system/bang-bang.h"

// #define MAIN_PROGRAM
#define CALIBRATE_HX711_PROGRAM

#define PIN_FAN 0
#define PIN_MIST 1
#define PIN_DHT 2
#define PIN_HEATER 3
#define PIN_DT_BREED 4
#define PIN_DT_YIELD 5
#define PIN_EN_PH 8
#define PIN_CLK 10

#ifndef MAIN_PROGRAM

WiFiConnection wifi("Subhanallah", "muhammadnabiyullah", "sibob-1");
WebServer server(80);
SensorData sensors;

DHTWrapper dht(PIN_DHT);
PHWrapper ph(PIN_EN_PH);
HX711 hx1;
HX711 hx2;
OneWire oneWire(2); // TODO: CHANGE PIN
DS18B20 ds(&oneWire);
ADS1115 ads(0x48);
ADSWrapper adsWrapper(ads);

void setup()
{
	Serial.begin(115200);
	wifi.begin();
	ElegantOTA.begin(&server);
	ElegantOTA.setAutoReboot(true);
	server.begin();

	sensors.temperature_air.status = dht.begin();
	sensors.weight_breed.status = weightBegin(hx1, PIN_DT_BREED, PIN_CLK);
	sensors.weight_yield.status = weightBegin(hx2, PIN_DT_YIELD, PIN_CLK);
	sensors.temperature_soil.status = soilTemperatureBegin(ds);
	sensors.ph.status = adsWrapper.adsBegin();

	// same sensor with temp air
	sensors.humidity_air.status = sensors.temperature_air.status;
	// same adc with ph
	sensors.humidity_soil.status = sensors.ph.status;
}

void loop()
{
	wifi.reconnect();

	sensors.temperature_air.value = dht.getTemperature();
	sensors.humidity_air.value = dht.getHumidity();
	sensors.weight_breed.value = weightReading(hx1);
	sensors.weight_yield.value = weightReading(hx2);
	sensors.temperature_soil.value = soilTemperatureReading(ds);
	sensors.humidity_soil.value = soilHumidityReading(adsWrapper);
	sensors.ph.value = ph.phReading(adsWrapper);

	static const BangBangController bang(
		BangBangConfig{40, 20, 90, 40}, // top temp, bot temp, top hum, bot hum
		ActuatorConfig{0, 1, 3});		// pin fan, pin mist, pin heater
	bang.control(sensors.temperature_soil.value, sensors.humidity_soil.value);

	wifi.postSupabase(sensors.toJsonDocument());

	ElegantOTA.loop();
	server.handleClient();
}

#endif

#ifndef CALIBRATE_HX711_PROGRAM

HX711 myScale;

//  adjust pins if needed.
uint8_t dataPin = 4;
uint8_t clockPin = 10;

void setup()
{
	Serial.begin(115200);
	Serial.println();
	Serial.println(__FILE__);
	Serial.print("HX711_LIB_VERSION: ");
	Serial.println(HX711_LIB_VERSION);
	Serial.println();

	myScale.begin(dataPin, clockPin);
}

void loop()
{
	calibrate();
}

void calibrate()
{
	Serial.println("\n\nCALIBRATION\n===========");
	Serial.println("remove all weight from the loadcell");
	//  flush Serial input
	while (Serial.available())
		Serial.read();

	Serial.println("and press enter\n");
	while (Serial.available() == 0)
		;

	Serial.println("Determine zero weight offset");
	//  average 20 measurements.
	myScale.tare(20);
	int32_t offset = myScale.get_offset();

	Serial.print("OFFSET: ");
	Serial.println(offset);
	Serial.println();

	Serial.println("place a weight on the loadcell");
	//  flush Serial input
	while (Serial.available())
		Serial.read();

	Serial.println("enter the weight in (whole) grams and press enter");
	uint32_t weight = 0;
	while (Serial.peek() != '\n')
	{
		if (Serial.available())
		{
			char ch = Serial.read();
			if (isdigit(ch))
			{
				weight *= 10;
				weight = weight + (ch - '0');
			}
		}
	}
	Serial.print("WEIGHT: ");
	Serial.println(weight);
	myScale.calibrate_scale(weight, 20);
	float scale = myScale.get_scale();

	Serial.print("SCALE:  ");
	Serial.println(scale, 6);

	Serial.print("\nuse scale.set_offset(");
	Serial.print(offset);
	Serial.print("); and scale.set_scale(");
	Serial.print(scale, 6);
	Serial.print(");\n");
	Serial.println("in the setup of your project");

	Serial.println("\n\n");
}

#endif