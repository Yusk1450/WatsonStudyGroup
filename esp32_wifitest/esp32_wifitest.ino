
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Arduino.h>
#include <WiFi.h>
#include <WifiMulti.h>

#include <HTTPClient.h>

const int btnPin = 34;
int btnStateOld = LOW;

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

WiFiMulti wifiMulti;

const char ssid[] = "ssid";              // SSID
const char pwd[] = "Password";          // パスワード

/* ----------------------------------------------------
 * 初期化処理
---------------------------------------------------- */
void setup()
{
    pinMode(btnPin, INPUT);
    Serial.begin(9600);
    delay(200);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    delay(1000);
    display.clearDisplay();

    // Wifi接続
    wifiMulti.addAP(ssid, pwd);
    while (wifiMulti.run() != WL_CONNECTED)
    {
        drawText("NETWORK\nCONNECTING");
        delay(500);
    }
    Serial.println("Wifi connecting...OK!");
    drawText("NETWORK\nOK");
}

/* ----------------------------------------------------
 * メイン処理
---------------------------------------------------- */
void loop()
{
    // ボタン状態
    int btnState = digitalRead(btnPin);

    // ボタンが押されたとき
    if (btnState == HIGH && btnStateOld == LOW)
    {
        Serial.println("Button Pressed");

        // 
        if (requestGetHttp("http://49.212.141.66/"))
        {
            drawText("SUCCESS!");
            Serial.println("Success!");
        }
        else
        {
            drawText("FAILED!");
            Serial.println("Failed: HTTP GET Request");
        }

        delay(10);      // チャタリング防止
    }

    btnStateOld = btnState;
}

void drawText(String text)
{
    display.clearDisplay();
    delay(200);

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    
    display.println(text);
    display.display();
}

boolean requestGetHttp(String url)
{
    boolean succeed = false;

    if (wifiMulti.run() == WL_CONNECTED)
    {
        HTTPClient http;
        http.begin(url);

        int httpCode = http.GET();
        Serial.println(httpCode);

        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                succeed = true;
            }
        }

        http.end();
    }

    delay(100);
    return succeed;
}

