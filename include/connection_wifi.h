#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

class WiFiConnection
{
public:
    WiFiConnection(const char *ssid, const char *password, const char *hostname);
    ~WiFiConnection();

    void begin();
    void reconnect();
    int post(JsonDocument payload);
    int postSupabase(JsonDocument payload);

private:
    const char *_supabase_url = "https://gothabjdasaphwzrjnto.supabase.co/rest/v1/";
    const char *_publishable_key = "sb_publishable_Dx3vXSh8qdQhM1Zi_V1MTQ_ifqdbX1o";
    const char *_sensorTable = "%5BSIBOB%5D%20sensor";
    const char *_ssid;
    const char *_password;
    const char *_hostname;
    WiFiClient _wifiClient;

    uint8_t _counterReset = 0;

    void _setupMDNS();
};

#endif // WIFI_H