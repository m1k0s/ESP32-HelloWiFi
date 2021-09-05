#include <Arduino.h>
#include <U8g2lib.h>
#include "checkwifi.h"

#define OLED_CLOCK 15
#define OLED_DATA 4
#define OLED_RESET 16

U8G2_SSD1306_128X64_NONAME_F_HW_I2C g_OLED(U8G2_R0, OLED_RESET, OLED_CLOCK, OLED_DATA);
uint32_t g_ScreenWidth;
uint32_t g_ScreenHeight;
uint32_t g_LineHeight;
uint32_t g_MaxCharWidth;

static const char WIFI_SSID[] = "Test-Fi"; //<<< CHANGE ME!
static const char WIFI_PASSPHRASE[] = "replaceme1234"; //<<< CHANGE ME!
static const char HOSTNAME[] = "heltec";

void setup()
{
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);

    g_OLED.begin();
    g_OLED.clear();

    g_ScreenWidth = g_OLED.getWidth();
    g_ScreenHeight = g_OLED.getHeight();

    g_OLED.setFont(u8g2_font_profont10_tf);
    g_LineHeight = g_OLED.getFontAscent() - g_OLED.getFontDescent();
    g_MaxCharWidth = g_OLED.getMaxCharWidth();

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
        fps = fps * 0.9f + (1000.0f / deltaMillis) * 0.1f;
    }

    g_OLED.clearBuffer();

    {
        extern void updateWiFiStatus(uint32_t deltaMillis);
        updateWiFiStatus(deltaMillis);
    }

    g_OLED.setCursor(g_ScreenWidth - 3 * g_MaxCharWidth, g_LineHeight);
    g_OLED.printf("%03.0f", fps);

    g_OLED.sendBuffer();

    // Cap @100ms per frame.
    // Work out how long everything up till now took in this frame and delay appropriately
    int32_t frameMillis = millis() - thisMillis;
    if(frameMillis < 100)
    {
        delay(100 - frameMillis);
    }
}

void drawConnectingAnim(uint8_t x, uint8_t y, uint32_t deltaMillis)
{
    const uint32_t CONNECTING_ANIM_DURATION_MILLIS = 500;
    static uint32_t connectingAnimTime = 0;
    static const char *const CONNECTING_ANIM[] = {".  ", ".. ", "...", "   "};

    const uint32_t BLINK_DURATION_MILLIS = CONNECTING_ANIM_DURATION_MILLIS / ARRAY_SIZE(CONNECTING_ANIM);
    static bool ledState = HIGH;
    static uint32_t ledMillis = 0;

    if (deltaMillis == -1)
    {
        // Reset internal state & return.
        connectingAnimTime = 0;
        ledState = LOW;
        ledMillis = 0;
        digitalWrite(LED_BUILTIN, ledState);
        return;
    }

    connectingAnimTime = (connectingAnimTime + deltaMillis) % CONNECTING_ANIM_DURATION_MILLIS;
    int index = (int)(connectingAnimTime * ARRAY_SIZE(CONNECTING_ANIM) / (float)CONNECTING_ANIM_DURATION_MILLIS);

    g_OLED.drawStr(0, g_LineHeight, CONNECTING_ANIM[index]);

    digitalWrite(LED_BUILTIN, ledState);
    ledMillis += deltaMillis;
    if (ledMillis >= BLINK_DURATION_MILLIS)
    {
        ledState = !ledState;
        ledMillis -= BLINK_DURATION_MILLIS;
    }
}

void drawWiFiStrength(uint8_t x, uint8_t y, int8_t rssi)
{
    // 4 bitmaps (16x6); each row is one uint16_t, that way we dont need to worry about endianness.
    static const uint16_t BARS[4][6] = {
        // 8421842184218421
        // ...........*.*.. 0014
        // ............*.*. 000a
        // ......*.*..*.*.. 0294
        // .......*.*..*.*. 014a
        // .*.*..*.*..*.*.. 5294
        // ..*.*..*.*..*.*. 294a
        {0x0014, 0x000a, 0x0294, 0x014a, 0x5294, 0x294a},
        // 8421842184218421
        // ...........*.*.. 0014
        // ............*.*. 000a
        // ......*.*..*.*.. 0294
        // .......*.*..*.*. 014a
        // .****.*.*..*.*.. 7a94
        // .****..*.*..*.*. 794a
        {0x0014, 0x000a, 0x0294, 0x014a, 0x7a94, 0x794a},
        // 8421842184218421
        // ...........*.*.. 0014
        // ............*.*. 000a
        // ......****.*.*.. 03d4
        // ......****..*.*. 03ca
        // .****.****.*.*.. 7bd4
        // .****.****..*.*. 7bca
        {0x0014, 0x000a, 0x03d4, 0x03ca, 0x7bd4, 0x7bca},
        // 8421842184218421
        // ...........****. 001e
        // ...........****. 001e
        // ......****.****. 03de
        // ......****.****. 03de
        // .****.****.****. 7bde
        // .****.****.****. 7bde
        {0x001e, 0x001e, 0x03de, 0x03de, 0x7bde, 0x7bde},
    };

    // RSSI should be -100 (worst) to 0 (best).
    // Consensous seems to be -50 or higher is essentially full signal, -90 or lower is very poor/no signal.
    // Remap to [-100, 0] to [0, 100].
    rssi = 100 + rssi;
    const uint8_t MAX_BARS = 4;
    // Depending on how many "bars" we want to display, find the nearest integer multiple
    // closest to 50 so the number of bars divides exactly.
    const uint8_t MAX_DBM = MAX_BARS * (uint8_t)(50 / MAX_BARS);
    // In _theory_ the range is [0, 100] but clamp just in case.
    if (rssi < 0) { rssi = 0; } if (rssi >= MAX_DBM) { rssi = MAX_DBM - 1; }
    const uint8_t DBM_PER_BAR = MAX_DBM / MAX_BARS;
    // RSSI -> number of bars we can visually distinguish.
    uint8_t bars = rssi / DBM_PER_BAR;
    // Pick the appropriate bitmap.
    const uint16_t *rows = BARS[bars];

    for (uint8_t h = 6; h > 0; --h, ++y, ++rows)
    {
        uint16_t row = *rows;
        // Start at bit 15 (left-most) and shift the mask right for each pixel in the row.
        uint16_t mask = 0x8000;
        for (uint8_t w = 16, _x = x; w > 0; --w, ++_x, mask >>= 1)
        {
            if (row & mask)
            {
                // Yuk; how slow is this?! At least it deals with display rotations...
                // !TODO! refactor if this becomes a problem.
                g_OLED.drawPixel(_x, y);
            }
        }
    }
}

void updateWiFiStatus(uint32_t deltaMillis)
{
    static bool connecting = false;

    switch (WiFiConnection::Status())
    {
    case WL_CONNECTED:
        if (connecting)
        {
            // Was drawing the connecting anim and are now connected; reset any internal state.
            connecting = false;
            drawConnectingAnim(0, 0, -1);
        }
        drawWiFiStrength(0, 0, WiFiConnection::RSSI(deltaMillis));
        {
            IPAddress localIP = WiFiConnection::LocalIP();
            g_OLED.setCursor(0, g_ScreenHeight);
            g_OLED.printf("%u.%u.%u.%u", localIP[0], localIP[1], localIP[2], localIP[3]);
        }
        break;
    default:
        // Track the fact that we started drawing the connecting anim so we can reset this when connected.
        connecting = true;
        drawConnectingAnim(0, g_LineHeight, deltaMillis);
        break;
    }
}