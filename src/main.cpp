#include <Arduino.h>
#include <WiFi.h>
#include <ElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
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

#define TOP_TEMP_SET 25
#define BOT_TEMP_SET 10
#define TOP_HUM_SET 90
#define BOT_HUM_SET 40

const char *ssid = "Bengkel Inovasi Indonesia";
const char *password = "EKSPEKTASI";
const char *hostname = "sibob-1";

#if defined(SIBOB_1)
WiFiConnection wifi(ssid, password, hostname);
#elif defined(SIBOB_2)
WiFiConnection wifi(ssid, password, hostname);
#endif
AsyncWebServer server(80);
SensorData sensors;

DHTWrapper dht(PIN_DHT);
PHWrapper ph(PIN_EN_PH);
HX711 hx1;
HX711 hx2;
OneWire oneWire(9);
DS18B20 ds(&oneWire);
ADS1115 ads(0x48);
ADSWrapper adsWrapper(ads);

String sensorSnapshotJson()
{
	JsonDocument doc = sensors.toJsonDocument();
	String payload;
	serializeJsonPretty(doc, payload);
	return payload;
}

int publishSensorSnapshot()
{
	JsonDocument doc = sensors.toJsonDocument();
	JsonDocument supabaseDoc;
	supabaseDoc["temperature_soil"] = sensors.temperature_soil.value;
	supabaseDoc["humidity_soil"] = sensors.humidity_soil.value;
	supabaseDoc["ph"] = sensors.ph.value;
	supabaseDoc["weight"] = doc["weight"];
	supabaseDoc["temperature_air"] = sensors.temperature_air.value;
	supabaseDoc["humidity_air"] = sensors.humidity_air.value;
	supabaseDoc["status"] = doc["status"];
	supabaseDoc["device_id"] = sensors.id;
	String payload;
	serializeJsonPretty(doc, payload);

	Serial.println(payload);
	WebSerial.println(payload);
	return wifi.postSupabase(supabaseDoc);
}

void setup()
{
	esp_task_wdt_init(30, true);
	esp_task_wdt_add(NULL);

	Serial.begin(115200);
	wifi.begin();
	ElegantOTA.begin(&server);
	ElegantOTA.setAutoReboot(true);
	WebSerial.begin(&server, "/webserial");

	pinMode(0, OUTPUT);
	pinMode(1, OUTPUT);
	pinMode(3, OUTPUT);

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(200, "application/json", sensorSnapshotJson()); });

	server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request)
			  { request->send(200, "text/plain", "Restarting now...");
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

	if (millis() > lastUpdate + 20000)
	{
#if defined(SIBOB_1)
		sensors.id = 1;
		sensors.weight_breed.value = weightReading(hx1);
		sensors.weight_yield.value = weightReading(hx2);
#elif defined(SIBOB_2)
		sensors.id = 2;
		sensors.weight_breed.value = weightReading(hx1);
		sensors.weight_yield.value = weightReading(hx2);
#endif
		WebSerial.printf("Weight: %.1f\n",
						 (sensors.weight_breed.value * 0.5 + sensors.weight_yield.value * 0.5));

		sensors.temperature_air.value = dht.getTemperature();
		sensors.humidity_air.value = dht.getHumidity();
		sensors.temperature_soil.value = soilTemperatureReading(ds);
		sensors.humidity_soil.value = soilHumidityReading(adsWrapper);
		sensors.ph.value = ph.phReading(adsWrapper);

		static const BangBangController bang(
			BangBangConfig{TOP_TEMP_SET,
						   BOT_TEMP_SET,
						   TOP_HUM_SET,
						   BOT_HUM_SET}, // top temp, bot temp, top hum, bot hum
			ActuatorConfig{0, 1, 3});	 // pin fan, pin mist, pin heater
		bang.control(sensors.temperature_soil.value, sensors.humidity_soil.value);

		int response = publishSensorSnapshot();
		WebSerial.printf("Supabase Res Code: %d\n", response);
		lastUpdate = millis();
	}

	ElegantOTA.loop();
}
