#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>

class WeatherManager {
  public:
    String detectedCity = "Zatec";
    float userLat = 50.3274;
    float userLon = 13.5449;
    
    String activeWeatherNames[5]; 
    String weatherDataArray[5];
    int numActiveLocations = 0;
    int currentWeatherLocationIndex = 0;

    // Hlavní metoda pro aktualizaci polohy, map a počasí
    void updateLocationAndWeather() {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin("http://ipapi.co/json/");
        int httpCode = http.GET();
        if (httpCode > 0) {
          String payload = http.getString();
          String cityStr = getJsonField(payload, "city");
          String latStr = getJsonField(payload, "latitude");
          String lonStr = getJsonField(payload, "longitude");

          if (cityStr.length() > 0 && cityStr.indexOf("<html>") == -1) {
            detectedCity = cityStr;
          }
          if (latStr.length() > 0 && lonStr.length() > 0) {
            userLat = latStr.toFloat();
            userLon = lonStr.toFloat();
          }
        }
        http.end();

        fetchTownsFromMap(userLat, userLon);
      }
    }

    // Vykreslení celého podmenu pro počasí přímo v knihovně
    void drawSubMenu(U8G2 &u8g2) {
      if (numActiveLocations > 0) {
        String fullData = weatherDataArray[currentWeatherLocationIndex];
        int commaIndex = fullData.indexOf(',');
        String tempOnly = fullData;
        if (commaIndex != -1) {
          tempOnly = fullData.substring(0, commaIndex); 
        }

        u8g2.setFont(u8g2_font_6x12_tr);
        u8g2.drawStr(0, 26, tempOnly.c_str());
        
        drawWeatherIcon(u8g2, 50, 36, fullData); 

        String locInfo = "Okolí 20km (" + String(currentWeatherLocationIndex + 1) + "/" + String(numActiveLocations) + ")";
        u8g2.setFont(u8g2_font_5x8_tf);
        u8g2.drawStr(0, 62, locInfo.c_str());
      } else {
        u8g2.setFont(u8g2_font_6x12_tr);
        u8g2.drawStr(0, 30, "Zadne lokace");
      }
    }

    // Obsluha otáčení kolečkem v podmenu počasí
    void handleScroll(int direction) {
      if (numActiveLocations > 0) {
        if (direction > 0) {
          currentWeatherLocationIndex = (currentWeatherLocationIndex + 1) % numActiveLocations;
        } else {
          currentWeatherLocationIndex = (currentWeatherLocationIndex - 1 + numActiveLocations) % numActiveLocations;
        }
      }
    }

  private:
    String getJsonField(String json, String key) {
      int keyIndex = json.indexOf("\"" + key + "\":");
      if (keyIndex == -1) return "";
      int startIdx = json.indexOf(":", keyIndex) + 1;
      while (startIdx < json.length() && (json[startIdx] == ' ' || json[startIdx] == '\"')) startIdx++;
      int endIdx = startIdx;
      while (endIdx < json.length() && json[endIdx] != '\"' && json[endIdx] != ',' && json[endIdx] != '}' && json[endIdx] != ' ') endIdx++;
      return json.substring(startIdx, endIdx);
    }

    void fetchTownsFromMap(float lat, float lon) {
      if (WiFi.status() != WL_CONNECTED) return;

      HTTPClient http;
      String query = "[out:json][timeout:10];(node(around:20000," + String(lat, 4) + "," + String(lon, 4) + ")[place~\"city|town|village\"];);out body 10;";
      
      http.begin("http://overpass-api.de/api/interpreter");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      int httpCode = http.POST("data=" + query);
      if (httpCode > 0) {
        String payload = http.getString();
        
        DynamicJsonDocument doc(4096);
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
          JsonArray elements = doc["elements"];
          numActiveLocations = 0;
          
          activeWeatherNames[0] = detectedCity;
          numActiveLocations = 1;

          for (JsonObject element : elements) {
            const char* name = element["tags"]["name"];
            if (name != nullptr && numActiveLocations < 5) {
              String townName = String(name);
              if (!townName.equalsIgnoreCase(detectedCity)) {
                bool duplicity = false;
                for(int i = 0; i < numActiveLocations; i++) {
                  if(activeWeatherNames[i].equalsIgnoreCase(townName)) duplicity = true;
                }
                if(!duplicity) {
                  activeWeatherNames[numActiveLocations] = townName;
                  numActiveLocations++;
                }
              }
            }
          }
        }
      }
      http.end();

      if (numActiveLocations == 0) {
        activeWeatherNames[0] = detectedCity;
        numActiveLocations = 1;
      }
      currentWeatherLocationIndex = 0;

      for(int i = 0; i < numActiveLocations; i++) {
        HTTPClient weatherHttp;
        String url = "http://wttr.in/" + activeWeatherNames[i] + "?format=%l:+%t,+%C";
        weatherHttp.begin(url);
        int code = weatherHttp.GET();
        if (code > 0) {
          weatherDataArray[i] = weatherHttp.getString();
        } else {
          weatherDataArray[i] = activeWeatherNames[i] + ": Chyba";
        }
        weatherHttp.end();
      }
    }

    void drawWeatherIcon(U8G2 &u8g2, int x, int y, String condition) {
      condition.toLowerCase();
      if (condition.indexOf("rain") != -1 || condition.indexOf("shower") != -1 || condition.indexOf("drizzle") != -1 || condition.indexOf("dest") != -1) {
        u8g2.drawBox(x+4, y+4, 16, 8);
        u8g2.drawCircle(x+4, y+8, 4);
        u8g2.drawCircle(x+20, y+8, 4);
        u8g2.drawLine(x+6, y+14, x+4, y+18);
        u8g2.drawLine(x+12, y+14, x+10, y+18);
        u8g2.drawLine(x+18, y+14, x+16, y+18);
      } else if (condition.indexOf("cloud") != -1 || condition.indexOf("overcast") != -1 || condition.indexOf("oblacno") != -1 || condition.indexOf("zatazeno") != -1) {
        u8g2.drawBox(x+4, y+6, 20, 10);
        u8g2.drawCircle(x+6, y+10, 6);
        u8g2.drawCircle(x+18, y+10, 6);
        u8g2.drawCircle(x+12, y+4, 6);
      } else if (condition.indexOf("sun") != -1 || condition.indexOf("clear") != -1 || condition.indexOf("jasno") != -1) {
        u8g2.drawDisc(x+12, y+10, 5);
        u8g2.drawLine(x+12, y+1, x+12, y+4);
        u8g2.drawLine(x+12, y+16, x+12, y+19);
        u8g2.drawLine(x+3, y+10, x+6, y+10);
        u8g2.drawLine(x+18, y+10, x+21, y+10);
        u8g2.drawLine(x+6, y+4, x+8, y+6);
        u8g2.drawLine(x+16, y+14, x+18, y+16);
        u8g2.drawLine(x+6, y+16, x+8, y+14);
        u8g2.drawLine(x+16, y+6, x+18, y+4);
      } else {
        u8g2.drawDisc(x+8, y+8, 4);
        u8g2.drawBox(x+8, y+10, 16, 8);
        u8g2.drawCircle(x+10, y+14, 4);
        u8g2.drawCircle(x+20, y+14, 4);
      }
    }
};

#endif