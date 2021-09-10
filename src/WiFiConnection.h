#pragma once

#include <WiFi.h>

class WiFiConnection
{
public:
    static const uint32_t RSSI_CHECK_TIMEOUT;
    static const uint32_t CONNECT_TIMEOUT;
    static const uint32_t RETRY_TIMEOUT;

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
};
