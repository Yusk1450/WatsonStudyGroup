
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

// LCD
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// Camera
#define PIC_PKT_LEN    33        //data length of each read, dont set this too big because ram is limited
#define PIC_FMT_VGA    7
#define PIC_FMT_QVGA   5
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0

#define PIC_FMT        PIC_FMT_QVGA

HardwareSerial mySerial2(2);
const byte cameraAddr = (CAM_ADDR << 5);  // addr
unsigned long picTotalLen = 0;            // picture length
#define QSTR_FLIR_LEN 40000
int qPostDatLen =QSTR_FLIR_LEN;
uint8_t qPostDat[QSTR_FLIR_LEN];
int qPostDatCntr=0;
int n = 0;

WiFiMulti wifiMulti;
String url = "http://yusk1450.sakura.ne.jp/watson_php/analyze.php";

const char ssid[] = "ssid";              // SSID
const char pwd[] = "Password";          // パスワード

/* ----------------------------------------------------
 * 初期化処理
---------------------------------------------------- */
void setup()
{
    Serial.begin(115200);

    mySerial2.begin(115200);
    drawText("CAMERA\nINITIALIZING");
    initialize();
    drawText("CAMERA\nOK");
    
    pinMode(btnPin, INPUT);
    delay(200);

    // LCD
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

    drawText("PRESS THE BUTTON");
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
        drawText("CAPTURE\nSTART");
        
        if (n == 0)
        {
            preCapture();
        }
        capture();
        getData();

        if (!httpPostColor(qPostDat,qPostDatCntr))
        {
            drawText("SEND DATA\nNG");
            Serial.println("POST COLOR ....NG!");
        }

        delay(10);      // チャタリング防止
    }

    n++;
    btnStateOld = btnState;
}

void drawText(String text)
{
    display.clearDisplay();
    delay(200);

    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
//    display.setTextColor(BLACK, WHITE);
    
    display.println(text);
    display.display();
}

boolean httpPostColor(uint8_t* postStr,int contLen)
{
    boolean rtnCode = false;

    if((wifiMulti.run() == WL_CONNECTED))
    {
        HTTPClient http;
        Serial.print("Color:[HTTP] begin...\n");

        http.begin(url);
        http.setTimeout(100000);
        http.addHeader("Host", "www.google.com");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        http.setUserAgent("Arduino Post Client");
        
        http.addHeader("Connection", "close");
        
        Serial.print("Color:[HTTP] POST...\n");
        int httpCode = http.POST(postStr,contLen);

        if(httpCode > 0)
        {
            Serial.printf("Color:[HTTP] POST... Response code: %d\n", httpCode);

            String payload = http.getString();
            Serial.print("payload: ");
            Serial.println(payload);
            drawText(payload);
            if(httpCode == HTTP_CODE_OK)
            {
                rtnCode = true;
            }
        }
        else
        {
            Serial.println(httpCode);
            Serial.printf("Color:[HTTP] POST... failed, error: %d %s\n", httpCode ,http.errorToString(httpCode).c_str());
        }
        http.end();
    }
    else
    {
      Serial.println("Color:wifiMulti.run() != WL_CONNECTED)");
    }

    delay(100);
    return rtnCode;
}

