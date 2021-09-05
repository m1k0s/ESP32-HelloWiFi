#pragma once

#include <WiFi.h>
#include <stdarg.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

const uint32_t WIFI_STATUS_CHECK_TIMEOUT = 100;
const uint32_t WIFI_RSSI_CHECK_TIMEOUT = 1000;
const uint32_t WIFI_CONNECT_TIMEOUT = 5000;
const uint32_t WIFI_RETRY_TIMEOUT = 10000;

class WiFiConnection
{
private:
    static wl_status_t status;
    static IPAddress localIP;

public:
    static void Init(const char* const ssid, const char* const passphrase, const char* const hostname = NULL);
    static int8_t RSSI(uint32_t deltaMillis);
    static void Reconnect();
    static inline wl_status_t Status() { return status; }
    static inline IPAddress LocalIP() { return localIP; }

private:
    static void EventHandler(WiFiEvent_t event);

private:
    static void Log(const char* format, ...);
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

void WiFiConnection::Reconnect()
{
    Log("WiFi reconnecting\n");
    WiFi.disconnect();
    WiFi.reconnect();
}

int8_t WiFiConnection::RSSI(uint32_t deltaMillis)
{
    static uint32_t rssiCheckTimeout = WIFI_RSSI_CHECK_TIMEOUT;
    static int8_t rssi = -128;

    rssiCheckTimeout += deltaMillis;
    if (rssiCheckTimeout >= WIFI_RSSI_CHECK_TIMEOUT)
    {
        rssiCheckTimeout = 0;
        rssi = WiFi.RSSI();
    }

    return rssi;
}

void WiFiConnection::EventHandler(WiFiEvent_t event)
{
    static const char* EVENT_NAMES[] = {
        "SYSTEM_EVENT_WIFI_READY",
        "SYSTEM_EVENT_SCAN_DONE",
        "SYSTEM_EVENT_STA_START",
        "SYSTEM_EVENT_STA_STOP",
        "SYSTEM_EVENT_STA_CONNECTED",
        "SYSTEM_EVENT_STA_DISCONNECTED",
        "SYSTEM_EVENT_STA_AUTHMODE_CHANGE",
        "SYSTEM_EVENT_STA_GOT_IP",
        "SYSTEM_EVENT_STA_LOST_IP",
        "SYSTEM_EVENT_STA_WPS_ER_SUCCESS",
        "SYSTEM_EVENT_STA_WPS_ER_FAILED",
        "SYSTEM_EVENT_STA_WPS_ER_TIMEOUT",
        "SYSTEM_EVENT_STA_WPS_ER_PIN",
        "SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP",
        "SYSTEM_EVENT_AP_START",
        "SYSTEM_EVENT_AP_STOP",
        "SYSTEM_EVENT_AP_STACONNECTED",
        "SYSTEM_EVENT_AP_STADISCONNECTED",
        "SYSTEM_EVENT_AP_STAIPASSIGNED",
        "SYSTEM_EVENT_AP_PROBEREQRECVED",
        "SYSTEM_EVENT_GOT_IP6",
        "SYSTEM_EVENT_ETH_START",
        "SYSTEM_EVENT_ETH_STOP",
        "SYSTEM_EVENT_ETH_CONNECTED",
        "SYSTEM_EVENT_ETH_DISCONNECTED",
        "SYSTEM_EVENT_ETH_GOT_IP"
    };
    if(event < ARRAY_SIZE(EVENT_NAMES))
    {
        Log("WiFi Event: %s\n", EVENT_NAMES[event]);
    }
    else
    {
        Log("WiFi Event: %d (unknown)\n", event);
    }

    status = WiFi.status();

    static const char* STATUS_NAMES[] = {
        "WL_IDLE_STATUS",
        "WL_NO_SSID_AVAIL",
        "WL_SCAN_COMPLETED",
        "WL_CONNECTED",
        "WL_CONNECT_FAILED",
        "WL_CONNECTION_LOST",
        "WL_DISCONNECTED"
    };
    if(status < ARRAY_SIZE(STATUS_NAMES))
    {
        Log("WiFi Status: %s\n", STATUS_NAMES[status]);
    }
    else
    {
        Log("WiFi Status: %d (unknown)\n", status);
    }

    switch (event)
    {
    case SYSTEM_EVENT_STA_GOT_IP:
        localIP = WiFi.localIP();
        Log("WiFi IP: %d.%d.%d.%d\n", localIP[0], localIP[1], localIP[2], localIP[3]);
        break;
    case SYSTEM_EVENT_STA_LOST_IP:
        localIP = INADDR_NONE;
        Log("WiFi IP: %d.%d.%d.%d\n", localIP[0], localIP[1], localIP[2], localIP[3]);
        break;
    default:
        break;
    }
}

void WiFiConnection::Log(const char* format, ...)
{
    if(Serial.availableForWrite())
    {
        char buffer[256];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        Serial.print(buffer);
    }
}