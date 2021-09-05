#pragma once

#include <WiFi.h>

const uint32_t WIFI_STATUS_CHECK_TIMEOUT = 100;
const uint32_t WIFI_STRENGTH_CHECK_TIMEOUT = 1000;
const uint32_t WIFI_CONNECT_TIMEOUT = 5000;
const uint32_t WIFI_RETRY_TIMEOUT = 10000;

class WiFiConnection
{
private:
    static wl_status_t status;
    static IPAddress localIP;

public:
    static void Init(const char* const ssid, const char* const passphrase, const char* const hostname = NULL);
    static int8_t Strength(uint32_t deltaMillis);
    static inline wl_status_t Status() { return status; }
    static inline IPAddress LocalIP() { return localIP; }

private:
    static void EventHandler(WiFiEvent_t event);
};

wl_status_t WiFiConnection::status = WL_IDLE_STATUS;
IPAddress WiFiConnection::localIP = INADDR_NONE;

void WiFiConnection::Init(const char* const ssid, const char* const passphrase, const char* const hostname)
{
    WiFi.disconnect(true);

    status = WL_IDLE_STATUS;
    localIP = INADDR_NONE;

    WiFi.mode(WIFI_STA);

    WiFi.onEvent(EventHandler);

    if(hostname != NULL)
    {
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
        WiFi.setHostname(hostname);
    }

    WiFi.begin(ssid, passphrase);
}

int8_t WiFiConnection::Strength(uint32_t deltaMillis)
{
    static uint32_t strengthCheckTimeout = WIFI_STRENGTH_CHECK_TIMEOUT;
    static int8_t strength = -128;

    strengthCheckTimeout += deltaMillis;
    if (strengthCheckTimeout >= WIFI_STRENGTH_CHECK_TIMEOUT)
    {
        strength = WiFi.RSSI();
        strengthCheckTimeout = 0;
    }

    return strength;
}

void WiFiConnection::EventHandler(WiFiEvent_t event)
{
    status = WiFi.status();

    switch (event)
    {
    case SYSTEM_EVENT_WIFI_READY:
        break;
    case SYSTEM_EVENT_SCAN_DONE:
        break;
    case SYSTEM_EVENT_STA_START:
        break;
    case SYSTEM_EVENT_STA_STOP:
        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        WiFi.disconnect();
        WiFi.reconnect();
        break;
    case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        localIP = WiFi.localIP();
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        localIP = INADDR_NONE;
        WiFi.reconnect();
        break;
    default:
        break;
    }
}