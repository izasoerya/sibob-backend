#include <Arduino.h>
#include <WiFi.h>
#include <ElegantOTA.h>
#include "models.h"
#include "connection_wifi.h"
#include "esp_task_wdt.h"

#include "sensor_reading/dht_reading.h"
#include "sensor_reading/ds18_reading.h"
#include "sensor_reading/hx711_reading.h"
#include "sensor_reading/ph_reading.h"
#include "sensor_reading/soil_hum_reading.h"
#include "control_system/bang-bang.h"

#define PIN_FAN 0
#define PIN_MIST 1
#define PIN_DHT 2
#define PIN_HEATER 3
#define PIN_DT_BREED 4
#define PIN_DT_YIELD 5
#define PIN_EN_PH 8
#define PIN_CLK 10

#if defined(SIBOB_1)
WiFiConnection wifi("Bengkel Inovasi Indonesia", "EKSPEKTASI", "sibob-1");
#elif defined(SIBOB_2)
WiFiConnection wifi("Bengkel Inovasi Indonesia", "EKSPEKTASI", "sibob-2");
#endif
WebServer server(80);
SensorData sensors;

DHTWrapper dht(PIN_DHT);
PHWrapper ph(PIN_EN_PH);
HX711 hx1;
HX711 hx2;
OneWire oneWire(9);
DS18B20 ds(&oneWire);
ADS1115 ads(0x48);
ADSWrapper adsWrapper(ads);

void setup()
{
	esp_task_wdt_init(30, true);
	esp_task_wdt_add(NULL);

	Serial.begin(115200);
	wifi.begin();
	ElegantOTA.begin(&server);
	ElegantOTA.setAutoReboot(true);

	pinMode(0, OUTPUT);
	pinMode(1, OUTPUT);
	pinMode(3, OUTPUT);

	server.on("/", []()
			  { 
				String buffer;
				serializeJsonPretty(sensors.toJsonDocument(), buffer);
				server.send(200, "application/json", buffer); });

	server.on("/restart", []()
			  { 
				server.send(200, "text/plain", "Restarting now..."); 
				ESP.restart(); });

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

uint64_t lastUpdate = 0;

void loop()
{
	wifi.reconnect();
	esp_task_wdt_reset();

	if (millis() > lastUpdate + 10000)
	{
#if defined(SIBOB_1)
		sensors.id = 1;
		sensors.weight_breed.value = (478804.59 - weightReading(hx1)) * 0.0075775;
		sensors.weight_yield.value = (-232862.92 - weightReading(hx2)) * 0.007118;
#else defined(SIBOB_2)
		sensors.id = 2;
		sensors.weight_breed.value = (478804.59 - weightReading(hx1)) * 0.0075775;
		sensors.weight_yield.value = (-232862.92 - weightReading(hx2)) * 0.007118;
#endif
		sensors.temperature_air.value = dht.getTemperature();
		sensors.humidity_air.value = dht.getHumidity();
		sensors.temperature_soil.value = soilTemperatureReading(ds);
		sensors.humidity_soil.value = soilHumidityReading(adsWrapper);
		sensors.ph.value = ph.phReading(adsWrapper);

		static const BangBangController bang(
			BangBangConfig{25, 10, 90, 40}, // top temp, bot temp, top hum, bot hum
			ActuatorConfig{0, 1, 3});		// pin fan, pin mist, pin heater
		bang.control(sensors.temperature_soil.value, sensors.humidity_soil.value);

		wifi.postSupabase(sensors.toJsonDocument());
		serializeJsonPretty(sensors.toJsonDocument(), Serial);
		lastUpdate = millis();
	}

	ElegantOTA.loop();
	server.handleClient();
}
