#ifndef HTML_H
#define HTML_H

#include <WebServer.h>
#include <WiFi.h>
#include <Preferences.h>

// Deklarace externích proměnných a struktur z hlavního souboru
struct SaverOption {
  int id;
  const char* name;
};

extern WebServer server;
extern bool wifiConfigMode;
extern String shoppingList[10];
extern int shoppingCount;
extern bool inScreenSaver;
extern bool inSubMenu;
extern int catFrameIndex;
extern int fishPosX;
extern int forcedSaverMode;         
extern unsigned long forcedSaverStartTime; 
extern Preferences preferences;
extern const Note happy[];
extern const uint16_t HAPPY_LENGTH;
extern const Note obed[];
extern const uint16_t OBED_LENGTH;
extern Buzzer buzzer;

// Externí pole spořičů definované v hlavním souboru
extern SaverOption saverItems[];
extern int numSaverItems;

// Společná hlavička a horní HTML menu
String getHtmlHeader(String title) {
  String html = "<!DOCTYPE html><html lang='cs'><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>" + title + "</title>";
  html += "<style>";
  html += "body { font-family: sans-serif; background: #f0f2f5; margin: 0; padding: 20px; color: #333; }";
  html += ".container { max-width: 400px; margin: auto; background: white; padding: 20px; border-radius: 12px; box-shadow: 0 4px 12px rgba(0,0,0,0.1); }";
  html += "h2 { color: #1a73e8; text-align: center; }";
  html += ".menu { display: flex; justify-content: space-around; background: #1a73e8; padding: 10px; border-radius: 6px; margin-bottom: 20px; }";
  html += ".menu a { color: white; text-decoration: none; font-weight: bold; font-size: 14px; }";
  html += ".menu a:hover { text-decoration: underline; }";
  html += ".input-group { display: flex; gap: 10px; margin-bottom: 20px; }";
  html += "input[type='text'], select, input[type='password'] { flex: 1; width: 100%; padding: 10px; border: 1px solid #ccc; border-radius: 6px; box-sizing: border-box; background: white; }";
  html += "label { display: block; margin-bottom: 5px; font-weight: bold; font-size: 14px; }";
  html += "button, .btn { background: #1a73e8; color: white; border: none; padding: 10px 15px; border-radius: 6px; cursor: pointer; font-weight: bold; text-decoration: none; display: inline-block; text-align: center; }";
  html += ".btn-block { display: block; width: 100%; box-sizing: border-box; margin-bottom: 10px; }";
  html += "ul { list-style: none; padding: 0; }";
  html += "li { display: flex; justify-content: space-between; align-items: center; background: #f8f9fa; margin-bottom: 6px; padding: 8px 12px; border-radius: 6px; }";
  html += ".del-btn { background: #ff4d4d; color: white; border: none; border-radius: 4px; padding: 4px 8px; cursor: pointer; }";
  html += "</style></head><body><div class='container'>";
  html += "<div class='menu'><a href='/'>Nákupní seznam</a><a href='/screensaver'>Spořič</a><a href='/melodies'>Melodie</a></div>";
  return html;
}

// Akce: Přehrání melodie z webu
void handlePlaySong() {
  if (server.hasArg("song")) {
    String song = server.arg("song");
    if (song == "happy") {
      buzzer.playMelody(happy, HAPPY_LENGTH, 120);
    } else if (song == "obed") {
      buzzer.playMelody(obed, OBED_LENGTH, 140);
    }
  }
  server.sendHeader("Location", "/melodies");
  server.send(303);
}

// 1. Nákupní seznam (hlavní stránka)
void handleRoot() {
  String html = getHtmlHeader("Nákupní seznam");
  html += "<h2>🛒 Nákupní seznam</h2>";
  html += "<form action='/add' method='POST' class='input-group'><input type='text' name='item' placeholder='Nová položka...' required><button type='submit'>Přidat</button></form><ul>";
  for(int i = 0; i < shoppingCount; i++) {
    if(shoppingList[i] != "") {
      html += "<li>" + shoppingList[i] + "<a href='/delete?id=" + String(i) + "'><button class='del-btn'>Smazat</button></a></li>";
    }
  }
  html += "</ul></div></body></html>";
  server.send(200, "text/html", html);
}

// 2. Ovládání spořiče s dynamicky generovanou roletkou z pole `saverItems`
void handleScreenSaverWeb() {
  String html = getHtmlHeader("Spořič obrazovky");
  html += "<h2>💤 Spořič obrazovky</h2>";
  html += "<p style='text-align: center;'>Vyberte spořič a spusťte jej na 1 minutu:</p>";
  
  html += "<form action='/trigger-saver' method='GET'>";
  html += "<div class='input-group'>";
  html += "<select name='mode'>";
  
  // Automatické vygenerování položek roletky
  for (int i = 0; i < numSaverItems; i++) {
    html += "<option value='" + String(saverItems[i].id) + "'>" + String(saverItems[i].name) + "</option>";
  }
  
  html += "</select>";
  html += "<button type='submit'>Spustit</button>";
  html += "</div>";
  html += "</form>";

  html += "<hr style='border:0; border-top:1px solid #eee; margin:20px 0;'>";
  html += "<a href='/trigger-saver?state=off' class='btn btn-block' style='background: #6c757d;'>Vypnout spořič</a>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

/// 3. Ovládání melodií s roletkou
void handleMelodiesWeb() {
  String html = getHtmlHeader("Přehrát melodii");
  html += "<h2>🎵 Melodie</h2>";
  html += "<p style='text-align: center;'>Vyberte melodii k přehrání:</p>";
  
  html += "<form action='/play' method='GET'>";
  html += "<div class='input-group'>";
  html += "<select name='song'>";
  html += "<option value='happy'>Happy</option>";
  html += "<option value='obed'>Oběd</option>";
  html += "</select>";
  html += "<button type='submit'>Přehrát</button>";
  html += "</div>";
  html += "</form>";

  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

// Stránka pro nastavení WiFi (AP režim)
void handleWifiConfigWeb() {
  int n = WiFi.scanNetworks();
  String html = getHtmlHeader("Nastavení WiFi");
  html += "<h2>📶 Nastavení WiFi</h2>";
  html += "<form action='/savewifi' method='POST'>";
  html += "<div class='input-group'><label>Vyber WiFi síť:</label><select name='ssid'>";
  if (n == 0) {
    html += "<option value=''>Žádné sítě nenalezeny</option>";
  } else {
    for (int i = 0; i < n; ++i) {
      html += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + " dBm)</option>";
    }
  }
  html += "</select></div>";
  html += "<div class='input-group'><label>Heslo:</label><input type='password' name='pass' placeholder='Heslo k WiFi'></div>";
  html += "<button type='submit' class='btn btn-block'>Uložit a restartovat</button>";
  html += "</form></div></body></html>";
  server.send(200, "text/html", html);
}

// Hlavní rozcestník pro kořenovou adresu "/"
void handleMain() {
  if (wifiConfigMode) {
    handleWifiConfigWeb();
  } else {
    handleRoot();
  }
}

// Akce: Přidání položky
void handleAdd() {
  if (server.hasArg("item") && shoppingCount < 10) {
    shoppingList[shoppingCount] = server.arg("item");
    shoppingCount++;
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// Akce: Smazání položky
void handleDelete() {
  if (server.hasArg("id")) {
    int id = server.arg("id").toInt();
    if (id >= 0 && id < shoppingCount) {
      for(int i = id; i < shoppingCount - 1; i++) {
        shoppingList[i] = shoppingList[i + 1];
      }
      shoppingCount--;
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// Akce: Spuštění/vypnutí spořiče z webu
void handleTriggerSaver() {
  if (server.hasArg("mode")) {
    forcedSaverMode = server.arg("mode").toInt();
    forcedSaverStartTime = millis();
    inScreenSaver = true;
    inSubMenu = false;
    catFrameIndex = 0;
    fishPosX = -50;
  } else if (server.hasArg("state")) {
    if (server.arg("state") == "on") {
      inScreenSaver = true;
      forcedSaverMode = 0; 
      inSubMenu = false;
      catFrameIndex = 0;
      fishPosX = -50;
    } else {
      inScreenSaver = false;
      forcedSaverMode = 0;
    }
  }
  server.sendHeader("Location", "/screensaver");
  server.send(303);
}

// Uložení WiFi parametrů
void handleSaveWifi() {
  if (server.hasArg("ssid")) {
    String newSsid = server.arg("ssid");
    String newPass = server.arg("pass");
    preferences.begin("wifi-config", false);
    preferences.putString("ssid", newSsid);
    preferences.putString("pass", newPass);
    preferences.end();
    server.send(200, "text/html", "<h3>WiFi uložena! Probíhá restart zařízení...</h3>");
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/html", "Chybí SSID!");
  }
}

#endif