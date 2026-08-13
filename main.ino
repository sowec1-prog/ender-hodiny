#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <time.h>
#include <Buzzer.h>
#include <Preferences.h>
#include "WeatherManager.h" 
#include "Cat_animation.h" 
#include "SunAnimation.h"
#include "FishAnimation.h"
#include "StarsAnimation.h"
#include "epd_bitmap_.h"
#include "pike.h"

#include "html.h" // Veškeré HTML a webové akce jsou nyní zde

//________melodie________
#include "mario_tone.h" 
#include "happy.h" 
#include "obed.h"

const int BUZZER_PIN = 10; 
Buzzer buzzer(BUZZER_PIN);

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 4, 6, 7, U8X8_PIN_NONE);

Preferences preferences;
WeatherManager weather; 
int lastLunchDay = -1;
const char* ntpServer = "pool.ntp.org";
String sunriseStr = "--:--";
String sunsetStr = "--:--";
WebServer server(80);

const int pinA = 8;
const int pinB = 9;
const int btnPin = 3;

// Nové menu bez spořičů a melodií, přidán Východ a západ slunce
const char *menuItems[] = {
  "1. Pocasi", 
  "2. Vychod a zapad", 
  "3. Kurzy men (CNB)", 
  "4. Nakupni seznam",
  "5. System / IP",
  "6. Zmena WiFi"
};
const int numItems = 6;
int selected = 0;
int scrollOffset = 0;
int pikeX = -128; // Začíná úplně vlevo mimo displej
bool inSubMenu = false;
int subSelected = 0;
int subScrollOffset = 0;
bool wifiConfigMode = false;

bool inScreenSaver = false;
int catFrameIndex = 0;
int epdFrameIndex = 0;
int fishPosX = -50;
unsigned long lastSaverAnimTime = 0;

// === MODULÁRNÍ SEZNAM SPOŘIČŮ / OBRÁZKŮ ===
SaverOption saverItems[] = {
  {1, "Kočka"},
  {2, "Štika"},        
  {3, "Východ slunce"},
  {4, "Hvězdy"},
  {5, "EPD Bitmapa"},   // 1. volný obrázek (epd_bitmap_.h)
  {6, "Pike Obraz"}     // 2. volný obrázek (pike.h)
};
int numSaverItems = 6;  // Celkem 6 položek v menu na webu

int forcedSaverMode = 0;         
unsigned long forcedSaverStartTime = 0;

String shoppingList[10] = {"Mleko", "Chleba", "Maslo", ""};
int shoppingCount = 3;

const char *melodyItems[] = {
  "Mario",
  "Happy",
  "Obed"
};
const int numMelodies = 3;

String cnbEurData = "Nacitani EUR...";
String cnbUsdData = "Nacitani USD...";
String sunData = "Nacitani...";
int sunriseHour = 6;   
int sunriseMinute = 0; 
int sunsetHour = 20;
int sunsetMinute = 0;
unsigned long lastDataFetch = 0;

unsigned long lastActivityTime = 0;
const unsigned long inactivityTimeout = 30000; 

unsigned long lastCheckTime = 0;
uint8_t old_a = HIGH;

void parseSunTimes(String payload) {
  int vIndex = payload.indexOf("Vychod:");
  if (vIndex != -1) {
    sunriseHour = payload.substring(vIndex + 8, vIndex + 10).toInt();
    sunriseMinute = payload.substring(vIndex + 11, vIndex + 13).toInt();
    char buf[10];
    sprintf(buf, "%02d:%02d", sunriseHour, sunriseMinute);
    sunriseStr = String(buf);
  }
  int zIndex = payload.indexOf("Zapad:");
  if (zIndex != -1) {
    sunsetHour = payload.substring(zIndex + 7, zIndex + 9).toInt();
    sunsetMinute = payload.substring(zIndex + 10, zIndex + 12).toInt();
    char buf[10];
    sprintf(buf, "%02d:%02d", sunsetHour, sunsetMinute);
    sunsetStr = String(buf);
  }
}

void fetchCNBData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://www.cnb.cz/cs/financni-trhy/devizovy-trh/kurzy-devizoveho-trhu/kurzy-devizoveho-trhu/denni_kurz.txt");
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      
      // Zpracování EUR
      int eurIndex = payload.indexOf("EUR");
      if (eurIndex != -1) {
        cnbEurData = payload.substring(eurIndex, eurIndex + 12);
      } else {
        cnbEurData = "EUR nenalezen";
      }

      // Zpracování USD
      int usdIndex = payload.indexOf("USD");
      if (usdIndex != -1) {
        cnbUsdData = payload.substring(usdIndex, usdIndex + 12);
      } else {
        cnbUsdData = "USD nenalezen";
      }
    } else {
      cnbEurData = "Chyba CNB";
      cnbUsdData = "Chyba CNB";
    }
    http.end();
  }
}

void fetchSunData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "http://wttr.in/" + weather.detectedCity + "?format=Vychod:+%S%0AZapad:+%s";
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      sunData = http.getString();
      parseSunTimes(sunData);
    } else {
      sunData = "Chyba pripojeni";
    }
    http.end();
  }
}

void drawStatusBar() {
  u8g2.setFont(u8g2_font_6x12_tr);
  int rssi = WiFi.RSSI();
  int bars = 0;
  if (rssi > -90) bars = 1;
  if (rssi > -75) bars = 2;
  if (rssi > -60) bars = 3;
  if (rssi > -45) bars = 4;

  for (int i = 0; i < 4; i++) {
    int h = (i + 1) * 2;
    if (i < bars) u8g2.drawBox(2 + (i * 3), 10 - h, 2, h);
    else u8g2.drawFrame(2 + (i * 3), 10 - h, 2, h);
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    mktime(&timeinfo);
    char timeDateStr[30];
    strftime(timeDateStr, sizeof(timeDateStr), "%H:%M  %d.%m.", &timeinfo);
    u8g2.drawStr(18, 10, timeDateStr);

    if (shoppingCount > 0) {
      u8g2.drawFrame(98, 3, 9, 7); 
      u8g2.drawLine(99, 4, 102, 6);  
      u8g2.drawLine(102, 6, 106, 4);  
      String countStr = String(shoppingCount);
      if (shoppingCount > 99) countStr = "99"; 
      u8g2.drawStr(109, 10, countStr.c_str());
    }
  } else {
    u8g2.drawStr(18, 10, "Cas nesynchronizován");
  }
  u8g2.drawHLine(0, 13, 128);
}

void drawDisplay() {
  u8g2.clearBuffer();

  if (wifiConfigMode) {
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 15, "REZIM ZMENY WIFI");
    u8g2.drawStr(0, 32, "Pripoj se na wifi:");
    u8g2.drawStr(0, 46, "ESP32-Nastaveni");
    u8g2.drawStr(0, 60, "IP: 192.168.4.1");
  } 
  else if (inScreenSaver) {
    struct tm timeinfo;
    String timeStr = "--:--";
    if (getLocalTime(&timeinfo)) {
      mktime(&timeinfo);
      char timeBuf[10];
      sprintf(timeBuf, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
      timeStr = String(timeBuf);
    }

    if (forcedSaverMode == 1) {
      const unsigned char* frame = (const unsigned char*)pgm_read_ptr(&cat_allArray[catFrameIndex]);
      u8g2.setDrawColor(1);
      u8g2.drawBitmap(0, 0, 128 / 8, 64, frame);
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.setCursor(48, 10);
      u8g2.print(timeStr);
    } 
    else if (forcedSaverMode == 2) {
      drawPike(u8g2, fishPosX, timeStr);
      u8g2.drawLine(0, 60, 127, 60);
    } 
    else if (forcedSaverMode == 3) {
      drawSunrise(u8g2, millis(), timeStr);
    } 
    else if (forcedSaverMode == 4) {
      drawStars(u8g2, timeStr);
    }
    else if (forcedSaverMode == 5) {
    // Načtení aktuálního snímku z PROGMEM pomocí správného názvu pole
    const unsigned char* frame = (const unsigned char*)pgm_read_ptr(&epd_bitmap_allArray[epdFrameIndex]);
    
    u8g2.setDrawColor(1);
    u8g2.drawBitmap(0, 0, 128 / 8, 64, frame);
    
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.setCursor(48, 10);
    u8g2.print(timeStr);

    // Posun na další snímek (přepínání animace)
    /* Můžete si vytvořit vlastní časovač nebo proměnnou pro změnu indexu (0 až 4)
    frameIndex++;
    if (frameIndex >= 5) {
      frameIndex = 0; // Po posledním snímku se vrátíme na začátek
    } */
}
    else if (forcedSaverMode == 6) {
      u8g2.setDrawColor(1);
      u8g2.drawBitmap(pikeX, 0, 128 / 8, 64, pike);
      pikeX += 4; 
      if (pikeX > 128) {
        pikeX = -128;
      }
      u8g2.setDrawColor(1); 
      u8g2.setFont(u8g2_font_ncenB08_tr);
      u8g2.setCursor(48, 10);
      u8g2.print(timeStr);
    }
    else {
      if (getLocalTime(&timeinfo)) {
        int currentMinutes = timeinfo.tm_hour * 60 + timeinfo.tm_min;
        int sunriseMinutes = sunriseHour * 60 + sunriseMinute;
        int sunsetMinutes = sunsetHour * 60 + sunsetMinute;

        if (currentMinutes >= sunriseMinutes && currentMinutes < sunriseMinutes + 5) {
          unsigned long elapsedMs = (currentMinutes - sunriseMinutes) * 60000UL + (timeinfo.tm_sec * 1000UL);
          drawSunrise(u8g2, millis() - elapsedMs, timeStr); 
        }
        else if (currentMinutes >= sunsetMinutes || currentMinutes < sunriseMinutes) {
          drawStars(u8g2, timeStr);
        }
        else if (timeinfo.tm_hour >= 14) {
             u8g2.setDrawColor(1);

      u8g2.drawBitmap(pikeX, 0, 128 / 8, 64, pike);

      pikeX += 4; 

      if (pikeX > 128) {

        pikeX = -128;

      }

      u8g2.setDrawColor(1); 

      u8g2.setFont(u8g2_font_ncenB08_tr);

      u8g2.setCursor(48, 10);

      u8g2.print(timeStr);
        }
        else {
          const unsigned char* frame = (const unsigned char*)pgm_read_ptr(&cat_allArray[catFrameIndex]);
          u8g2.setDrawColor(1);
          u8g2.drawBitmap(0, 0, 128 / 8, 64, frame);
          u8g2.setFont(u8g2_font_ncenB08_tr);
          u8g2.setCursor(48, 10);
          u8g2.print(timeStr);
        }
      } else {
        const unsigned char* frame = (const unsigned char*)pgm_read_ptr(&cat_allArray[catFrameIndex]);
        u8g2.setDrawColor(1);
        u8g2.drawBitmap(0, 0, 128 / 8, 64, frame);
      }
    }
  } 
  else {
    drawStatusBar();

    if (!inSubMenu) {
      int visibleRows = 4;
      if (selected < scrollOffset) scrollOffset = selected;
      else if (selected >= scrollOffset + visibleRows) scrollOffset = selected - visibleRows + 1;

      for(int i = 0; i < visibleRows; i++) {
        int itemIndex = scrollOffset + i;
        if (itemIndex >= numItems) break;

        int yPos = 27 + (i * 12);
        if(itemIndex == selected) {
          u8g2.drawStr(0, yPos, ">");
          u8g2.drawBox(10, yPos - 10, 118, 12);
          u8g2.setDrawColor(0);
          u8g2.drawStr(12, yPos, menuItems[itemIndex]);
          u8g2.setDrawColor(1);
        } else {
          u8g2.drawStr(12, yPos, menuItems[itemIndex]);
        }
      }
    } else {
      u8g2.setFont(u8g2_font_6x12_tr);
      if (selected == 0) {
        weather.drawSubMenu(u8g2);
      } else if (selected == 1) {
        // Podmenu pro Východ a západ slunce
        u8g2.drawStr(0, 25, "--- VYCHOD / ZAPAD ---");
        String sRise = "Vychod: " + sunriseStr;
        String sSet = "Zapad:  " + sunsetStr;
        u8g2.drawStr(0, 42, sRise.c_str());
        u8g2.drawStr(0, 58, sSet.c_str());
      } else if (selected == 2) {
        // Podmenu pro Kurzy měn (EUR + USD)
        u8g2.drawStr(0, 25, "--- KURZY MEN (CNB) ---");
        u8g2.drawStr(0, 42, cnbEurData.c_str());
        u8g2.drawStr(0, 58, cnbUsdData.c_str());
      } else if (selected == 3) {
        // Nákupní seznam
        u8g2.drawStr(0, 25, "--- NAKUPNI SEZNAM ---");
        if (shoppingCount == 0) {
          u8g2.drawStr(0, 45, "Seznam je prazdny.");
        } else {
          int visibleSubRows = 3;
          if (subSelected < subScrollOffset) subScrollOffset = subSelected;
          else if (subSelected >= subScrollOffset + visibleSubRows) subScrollOffset = subSelected - visibleSubRows + 1;

          for(int i = 0; i < visibleSubRows; i++) {
            int itemIndex = subScrollOffset + i;
            if (itemIndex >= shoppingCount) break;
            int yPos = 38 + (i * 10);
            if(itemIndex == subSelected) {
              u8g2.drawStr(0, yPos, ">");
              u8g2.drawBox(10, yPos - 9, 118, 10);
              u8g2.setDrawColor(0);
              u8g2.drawStr(12, yPos, shoppingList[itemIndex].c_str());
              u8g2.setDrawColor(1);
            } else {
              u8g2.drawStr(12, yPos, shoppingList[itemIndex].c_str());
            }
          }
        }
      } else if (selected == 4) {
        // Systém / IP
        u8g2.drawStr(0, 25, "--- SYSTEM ---");
        String ipStr = "IP: " + WiFi.localIP().toString();
        u8g2.drawStr(0, 40, ipStr.c_str());
      } else if (selected == 5) {
        // Změna WiFi
        u8g2.drawStr(0, 25, "--- ZMENA WIFI ---");
        u8g2.drawStr(0, 42, "Stiskni pro start");
        u8g2.drawStr(0, 56, "AP: ESP32-Nastaveni");
      }
    }
  }
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200); 
  delay(1000);          

  u8g2.begin();
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  pinMode(btnPin, INPUT_PULLUP);
  buzzer.begin(); 

  String savedSsid = "Vodafone-2g";
  String savedPass = "Stehlikova11";

  preferences.begin("wifi-config", true);
  if (preferences.isKey("ssid")) {
    savedSsid = preferences.getString("ssid");
  }
  if (preferences.isKey("pass")) {
    savedPass = preferences.getString("pass");
  }
  preferences.end();
  WiFi.mode(WIFI_STA);
  WiFi.begin(savedSsid.c_str(), savedPass.c_str());
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    delay(500);
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi připojena!");
    Serial.print("IP adresa zařízení je: ");
    Serial.println(WiFi.localIP());

    configTime(7200, 0, ntpServer);
    weather.updateLocationAndWeather(); 
    fetchCNBData();
    fetchSunData();
  } else {
    Serial.println("Nepodařilo se připojit k WiFi!");
  }

  server.on("/", handleMain);
  server.on("/screensaver", handleScreenSaverWeb);
  server.on("/trigger-saver", handleTriggerSaver);
  server.on("/melodies", handleMelodiesWeb);
  server.on("/play", handlePlaySong);
  server.on("/add", HTTP_POST, handleAdd);
  server.on("/delete", handleDelete);
  server.on("/savewifi", HTTP_POST, handleSaveWifi);
  server.begin();
  lastActivityTime = millis();
  drawDisplay();
}

void loop() {
  server.handleClient();

  if (forcedSaverMode != 0 && (millis() - forcedSaverStartTime > 60000)) {
    forcedSaverMode = 0;
    inScreenSaver = false;
    drawDisplay();
  }

  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    if (timeinfo.tm_hour == 12 && timeinfo.tm_min == 0) {
      if (timeinfo.tm_mday != lastLunchDay) {
        lastLunchDay = timeinfo.tm_mday; 
        buzzer.playMelody(obed, OBED_LENGTH, 140); 
      }
    }
  }

  if (!wifiConfigMode) {
    if (millis() - lastDataFetch > 600000) {
      weather.updateLocationAndWeather();
      fetchCNBData();
      fetchSunData();
      lastDataFetch = millis();
    }
  }

  if (!wifiConfigMode && !inScreenSaver && (millis() - lastActivityTime > inactivityTimeout)) {
    inScreenSaver = true;
    inSubMenu = false;
    catFrameIndex = 0;
    fishPosX = -50;
    drawDisplay();
  }

  if (inScreenSaver && (millis() - lastSaverAnimTime > 500)) {
    lastSaverAnimTime = millis();
    catFrameIndex = (catFrameIndex + 1) % 4;
    epdFrameIndex = (epdFrameIndex + 1) % 5;
    fishPosX += 2;
    if (fishPosX > 128) fishPosX = -50;
    drawDisplay();
  }

  if (millis() - lastCheckTime >= 2) {
    lastCheckTime = millis();
    uint8_t a = digitalRead(pinA);
    if (a != old_a) {
      if (a == LOW) {
        lastActivityTime = millis();
        if (inScreenSaver) {
          inScreenSaver = false; 
          forcedSaverMode = 0;   
        } else {
          if (digitalRead(pinB) == HIGH) {
            if (!inSubMenu) {
              selected = (selected + 1) % numItems;
            } else {
              if (selected == 0) {
                weather.handleScroll(1); 
              } else if (selected == 3 && shoppingCount > 0) {
                subSelected = (subSelected + 1) % shoppingCount;
              }
            }
          } else {
            if (!inSubMenu) {
              selected = (selected - 1 + numItems) % numItems;
            } else {
              if (selected == 0) {
                weather.handleScroll(-1); 
              } else if (selected == 3 && shoppingCount > 0) {
                subSelected = (subSelected - 1 + shoppingCount) % shoppingCount;
              }
            }
          }
        }
        drawDisplay();
      }
      old_a = a;
    }
  }

  if (digitalRead(btnPin) == LOW) {
    delay(50);
    if (digitalRead(btnPin) == LOW) {
      lastActivityTime = millis();
      if (inScreenSaver) {
        inScreenSaver = false; 
        forcedSaverMode = 0;     
      } else {
        if (!inSubMenu) {
          inSubMenu = true;
          subSelected = 0;
          subScrollOffset = 0;
        } else {
          if (selected == 3 && shoppingCount > 0) {
            for(int i = subSelected; i < shoppingCount - 1; i++) {
              shoppingList[i] = shoppingList[i + 1];
            }
            shoppingCount--;
            if (subSelected >= shoppingCount && shoppingCount > 0) subSelected = shoppingCount - 1;
          } 
          else if (selected == 5) {
            wifiConfigMode = true;
            WiFi.disconnect();
            WiFi.softAP("ESP32-Nastaveni");
          } 
          else {
            inSubMenu = false;
          }
        }
      }
      drawDisplay();
      while(digitalRead(btnPin) == LOW);
    }
  }
}
