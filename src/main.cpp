#include <Arduino.h>
#include "LedBuiltin.h"
#include "Display.h"
#include "WiFiConnection.h"
#include "Logger.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define POWER_DETECTION 37

Display g_Display = Display();

uint32_t g_ScreenWidth;
uint32_t g_ScreenHeight;
uint32_t g_ScreenCenterX;
uint32_t g_ScreenCenterY;
uint32_t g_LineHeight;
uint32_t g_MaxCharWidth;

static const char WIFI_SSID[] = "Test-Fi"; //<<< CHANGE ME!
static const char WIFI_PASSPHRASE[] = "replaceme1234"; //<<< CHANGE ME!
static const char HOSTNAME[] = "heltec";

#define UTC_OFFSET_SECS 0
#define DST_OFFSET_SECS 3600
static const char NTP_SERVER[] = "uk.pool.ntp.org";

void setup()
{
    Logger::Init(9600);
    delay(1000);
    Logger::Log("HeapSize: %u\n", ESP.getHeapSize());
    Logger::Log("FreeHeap: %u\n", ESP.getFreeHeap());
    Logger::Log("MinFreeHeap: %u\n", ESP.getMinFreeHeap());
    Logger::Log("MaxAllocHeap: %u\n", ESP.getMaxAllocHeap());
    Logger::Log("ChipRevision: %u\n", ESP.getChipRevision());
    Logger::Log("ChipModel: %s\n", ESP.getChipModel());
    Logger::Log("ChipCores: %u\n", ESP.getChipCores());
    Logger::Log("CpuFreqMHz: %u\n", ESP.getCpuFreqMHz());
    Logger::Log("SdkVersion: %s\n", ESP.getSdkVersion());
    Logger::Log("FlashChipSize: %u\n", ESP.getFlashChipSize());
    Logger::Log("FlashChipSpeed: %u\n", ESP.getFlashChipSpeed());
    Logger::Log("FlashChipMode: %s\n", ESP.getFlashChipMode());
    Logger::Log("SketchSize: %u\n", ESP.getSketchSize());
    Logger::Log("SketchMD5: %s\n", ESP.getSketchMD5().c_str());
    Logger::Log("FreeSketchSpace: %u\n", ESP.getFreeSketchSpace());
    Logger::Log("EfuseMac: %016lx\n", ESP.getEfuseMac());

    adcAttachPin(13);
    analogSetClockDiv(255); // 1338mS

    LedBuiltin::Init();

    g_Display.Init();

    g_ScreenWidth = g_Display.Width();
    g_ScreenHeight = g_Display.Height();
    g_ScreenCenterX = g_ScreenWidth / 2;
    g_ScreenCenterY = g_ScreenHeight / 2;

    g_LineHeight = g_Display.FontHeight();
    g_MaxCharWidth = g_Display.FontMaxCharWidth();

    WiFiConnection::Init(WIFI_SSID, WIFI_PASSPHRASE, HOSTNAME);
}

void loop()
{
    static uint32_t lastMillis = 0;
    uint32_t thisMillis = millis();
    uint32_t deltaMillis = thisMillis - lastMillis;
    lastMillis = thisMillis;

    static float fps = 0.0f;
    if (deltaMillis > 0)
    {
        const uint32_t FPS_AVERAGE_FRAME_COUNT = 10;
        const float FPS_ONE_FRAME_WEIGHT = 1.0f / FPS_AVERAGE_FRAME_COUNT;
        fps = fps * (1.0f - FPS_ONE_FRAME_WEIGHT) + (1000.0f / deltaMillis) * FPS_ONE_FRAME_WEIGHT;
    }

    g_Display.Clear();

    {
        extern void updateWiFiStatus(uint32_t deltaMillis);
        updateWiFiStatus(deltaMillis);
    }

    {
        extern void updatePowerStatus(uint32_t deltaMillis);
        updatePowerStatus(deltaMillis);
    }

    if (WiFiConnection::Status() == WL_CONNECTED)
    {
        extern void updateTime(uint32_t deltaMillis);
        updateTime(deltaMillis);
    }

    g_Display.PrintF(g_ScreenWidth - 3 * g_MaxCharWidth, 0, "%03.0f", fps);

    g_Display.SendBuffer();

    const uint32_t FRAME_TIME_CAP_MILLIS = 50;
    // Work out how long everything up till now took in this frame and delay appropriately
    uint32_t frameMillis = millis() - thisMillis;
    if (frameMillis < FRAME_TIME_CAP_MILLIS)
    {
        delay(FRAME_TIME_CAP_MILLIS - frameMillis);
    }
}

void drawConnectingAnim(uint8_t x, uint8_t y, uint32_t deltaMillis)
{
    const uint32_t CONNECTING_ANIM_DURATION_MILLIS = 500;
    static uint32_t connectingAnimTime = 0;
    static const char *const CONNECTING_ANIM[] = {".  ", ".. ", "...", "   "};

    const uint32_t BLINK_DURATION_MILLIS = CONNECTING_ANIM_DURATION_MILLIS / ARRAY_SIZE(CONNECTING_ANIM);
    static bool ledHigh = true;
    static uint32_t ledMillis = 0;

    if (deltaMillis == -1)
    {
        // Reset internal state & return.
        connectingAnimTime = 0;
        ledHigh = false;
        ledMillis = 0;
        LedBuiltin::Set(ledHigh);
        return;
    }

    connectingAnimTime = (connectingAnimTime + deltaMillis) % CONNECTING_ANIM_DURATION_MILLIS;
    int index = (int)(connectingAnimTime * ARRAY_SIZE(CONNECTING_ANIM) / (float)CONNECTING_ANIM_DURATION_MILLIS);

    g_Display.PrintF(0, g_LineHeight, CONNECTING_ANIM[index]);

    LedBuiltin::Set(ledHigh);
    ledMillis += deltaMillis;
    if (ledMillis >= BLINK_DURATION_MILLIS)
    {
        ledHigh = !ledHigh;
        ledMillis -= BLINK_DURATION_MILLIS;
    }
}

void drawBitmap16(uint8_t x, uint8_t y, uint8_t cnt, uint8_t h, const uint16_t *rows)
{
    uint8_t w = cnt * 16;
    for (; h > 0; --h, ++y, ++rows)
    {
        uint16_t row = *rows;
        // Start at bit 15 (left-most) and shift the mask right for each pixel in the row.
        uint16_t mask = 0x8000;
        for (uint8_t _w = w, _x = x; _w > 0; --_w, ++_x, mask >>= 1)
        {
            if (row & mask)
            {
                // Yuk; how slow is this?! At least it deals with display rotations...
                // !TODO! refactor if this becomes a problem.
                g_Display.DrawPixel(_x, y);
            }
        }
    }
}

void drawWiFiNoSignal(uint8_t x, uint8_t y)
{
    static const uint16_t NO_SIGNAL[7] = {
        0b0000101110000000,
        0b0000010001000000,
        0b0000101000100000,
        0b0000100100100000,
        0b0000100010100000,
        0b0000010001000000,
        0b0000001110100000
    };
    drawBitmap16(x, y, 1, 7, NO_SIGNAL);
}

void drawWiFiStrength(uint8_t x, uint8_t y, int8_t rssi)
{
    // 4 bitmaps (16x6); each row is one uint16_t, that way we dont need to worry about endianness.
    static const uint16_t BARS[4][6] = {
        {
            0b0000000000010100,
            0b0000000000001010,
            0b0000001010010100,
            0b0000000101001010,
            0b0101001010010100,
            0b0010100101001010
        },
        {
            0b0000000000010100,
            0b0000000000001010,
            0b0000001010010100,
            0b0000000101001010,
            0b0111101010010100,
            0b0111100101001010
        },
        {
            0b0000000000010100,
            0b0000000000001010,
            0b0000001111010100,
            0b0000001111001010,
            0b0111101111010100,
            0b0111101111001010
        },
        {
            0b0000000000011110,
            0b0000000000011110,
            0b0000001111011110,
            0b0000001111011110,
            0b0111101111011110,
            0b0111101111011110
        }
    };

    // RSSI should be -100 (worst) to 0 (best).
    // Consensous seems to be -50 or higher is essentially full signal, -90 or lower is very poor/no signal.
    // Remap to [-100, 0] to [0, 100].
    rssi = 100 + rssi;
    const uint8_t MAX_BARS = 4;
    // Depending on how many "bars" we want to display, find the nearest integer multiple
    // closest to 50 so the number of bars divides exactly.
    const uint8_t MAX_DBM = MAX_BARS * (uint8_t)(50 / MAX_BARS);
    // Clamp to [0, MAX_DBM).
    if (rssi < 0) { rssi = 0; } else if (rssi >= MAX_DBM) { rssi = MAX_DBM - 1; }
    const uint8_t DBM_PER_BAR = MAX_DBM / MAX_BARS;
    // RSSI -> number of bars we can visually distinguish.
    uint8_t bars = rssi / DBM_PER_BAR;
    // Pick & draw the appropriate bitmap.
    drawBitmap16(x, y, 1, 6, BARS[bars]);
}

void updateWiFiStatus(uint32_t deltaMillis)
{
    static wl_status_t lastStatus = (wl_status_t)-1;
    static bool connected = false;
    static uint32_t connectTimeout = 0;
    static uint32_t retryTimeout = 0;

    wl_status_t status = WiFiConnection::Status();
    switch (status)
    {
    case WL_CONNECTED:
        if (lastStatus != WL_CONNECTED)
        {
            // Just connected; reset internal state.
            connected = true;
            connectTimeout = 0;
            retryTimeout = 0;
            drawConnectingAnim(0, 0, -1);

            // Configure NTP.
            configTime(UTC_OFFSET_SECS, DST_OFFSET_SECS, NTP_SERVER);
        }
        drawWiFiStrength(0, 0, WiFiConnection::RSSI(deltaMillis));
        {
            IPAddress localIP = WiFiConnection::LocalIP();
            g_Display.PrintF(0, g_ScreenHeight, "%u.%u.%u.%u", localIP[0], localIP[1], localIP[2], localIP[3]);
        }
        break;
    default:
        if (connected)
        {
            // If previously connected; wait the retry timeout.
            retryTimeout += deltaMillis;
            if (retryTimeout >= WiFiConnection::RETRY_TIMEOUT)
            {
                retryTimeout = 0;
                connected = false;
                drawConnectingAnim(0, 0, -1);
                WiFiConnection::Reconnect();
            }
        }
        else
        {
            // Not previously connected; wait the connect timeout.
            connectTimeout += deltaMillis;
            if (connectTimeout >= WiFiConnection::CONNECT_TIMEOUT)
            {
                // Connection timeout; enter the retry state.
                connected = true;
                connectTimeout = 0;
                retryTimeout = 0;
                drawConnectingAnim(0, 0, -1);
            }
        }
        if (connectTimeout > 0)
        {
            drawConnectingAnim(0, g_LineHeight, deltaMillis);
        }
        else
        {
            drawWiFiNoSignal(0, 0);
        }
        break;
    }

    lastStatus = status;
}

void updatePowerStatus(uint32_t deltaMillis)
{
    static float power = 0.0f;

    const float MAX_POWER = 2480.0f;
    const float MIN_POWER = 2200.0f;
    uint16_t sampleRaw = analogRead(POWER_DETECTION);
    float sample = ((float)sampleRaw - MIN_POWER) / (MAX_POWER - MIN_POWER);
    if (sample < 0.0f) sample = 0.0f; else if (sample > 1.0f) sample = 1.0f;

    const uint32_t POWER_AVERAGE_SAMPLE_COUNT = 10;
    const float POWER_ONE_SAMPLE_WEIGHT = 1.0f / POWER_AVERAGE_SAMPLE_COUNT;
    power = power * (1.0f - POWER_ONE_SAMPLE_WEIGHT) + sample * POWER_ONE_SAMPLE_WEIGHT;

    const uint8_t X = 18;
    const uint8_t Y = 0;
    const uint8_t WIDTH = 16;
    const uint8_t HEIGHT = 6;
    g_Display.DrawRect(X, Y, WIDTH, HEIGHT);
    g_Display.DrawLine(X + WIDTH, Y + 2, X + WIDTH, HEIGHT - 2 - 2);
    g_Display.FillRect(X + 1, Y + 1, (WIDTH - 2 + 0.5f) * power, HEIGHT - 2);

    g_Display.PrintF(g_ScreenWidth - 4 * g_MaxCharWidth, g_ScreenHeight, "%3.0f%%", sample * 100.0f);
}

void updateTime(uint32_t deltaMillis)
{
    static const char *DAY_OF_WEEK[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    static const char *MONTH[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    tm ti;
    getLocalTime(&ti);
    timeval tv;
    gettimeofday(&tv, NULL);

    g_Display.PrintF(
        g_ScreenCenterX - g_MaxCharWidth * 10 / 2,
        g_ScreenCenterY + g_LineHeight / 2,
        "%02d:%02d:%02d.%u",
        ti.tm_hour,
        ti.tm_min,
        ti.tm_sec,
        (uint32_t)(tv.tv_usec / 100000)
    );

    g_Display.PrintF(
        g_ScreenCenterX - g_MaxCharWidth * 15 / 2,
        g_ScreenCenterY - g_LineHeight / 2,
        "%3s %2d %3s %4d",
        DAY_OF_WEEK[ti.tm_wday],
        ti.tm_mday,
        MONTH[ti.tm_mon],
        ti.tm_year + 1900
    );
}
