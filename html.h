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

// Společná hlavička a horní HTML menu s fixem pro statický směr kočky
String getHtmlHeader(String title) {
  String html = "<!DOCTYPE html><html lang='cs'><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>" + title + "</title>";
  html += "<style>";
  html += "body { font-family: 'Georgia', serif; background: linear-gradient(180deg, #0f2027, #203a43, #2c5364); color: #d1e8e2; margin: 0; padding: 20px; }";
  html += ".container { max-width: 400px; margin: auto; background: rgba(44, 62, 80, 0.85); padding: 20px; border-radius: 15px; border: 2px solid #5d4037; box-shadow: 0 8px 20px rgba(0,0,0,0.5); box-sizing: border-box; }";
  html += "h2 { color: #ffd700; text-align: center; text-shadow: 1px 1px 2px #000; }";
  html += ".menu { display: flex; justify-content: space-around; background: #2c3e50; padding: 10px; border-radius: 8px; margin-bottom: 10px; border: 1px solid #8b4513; }";
  html += ".menu a { color: #87CEEB; text-decoration: none; font-weight: bold; font-size: 14px; }";
  html += ".menu a:hover { color: #fff; text-decoration: underline; }";
  
// Opravený směr – kočka jde vždy hlavou dopředu
  html += ".cat-track { width: 100%; height: 32px; overflow: hidden; position: relative; margin-bottom: 10px; }";
  html += ".walking-cat { position: absolute; top: 0px; font-size: 24px; animation: walkCat 8s infinite linear; }";
  html += "@keyframes walkCat { "
          "0% { left: 0px; transform: scaleX(-1); } "
          "49.9% { left: calc(100% - 30px); transform: scaleX(-1); } "
          "50% { left: calc(100% - 30px); transform: scaleX(1); } "
          "99.9% { left: 0px; transform: scaleX(1); } "
          "100% { left: 0px; transform: scaleX(-1); } "
          "}";

  html += ".input-group { display: flex; gap: 10px; margin-bottom: 20px; }";
  html += "input[type='text'], select, input[type='password'] { flex: 1; width: 100%; padding: 10px; border: 1px solid #5d4037; border-radius: 6px; box-sizing: border-box; background: #1a1a1a; color: #fff; }";
  html += "label { display: block; margin-bottom: 5px; font-weight: bold; font-size: 14px; color: #d1e8e2; }";
  html += "button, .btn { background: linear-gradient(135deg, #2980b9, #2c3e50); color: #ffd700; border: 2px solid #ffd700; padding: 10px 15px; border-radius: 20px; cursor: pointer; font-weight: bold; text-decoration: none; display: inline-block; text-align: center; transition: 0.3s; box-shadow: 0 4px 10px rgba(0,0,0,0.3); }";
  html += "button:hover, .btn:hover { background: linear-gradient(135deg, #3498db, #1abc9c); color: #fff; transform: scale(1.03); box-shadow: 0 0 12px #ffd700; }";
  html += ".btn-block { display: block; width: 100%; box-sizing: border-box; margin-bottom: 10px; }";
  html += "ul { list-style: none; padding: 0; }";
  html += "li { display: flex; justify-content: space-between; align-items: center; background: rgba(255,255,255,0.05); margin-bottom: 6px; padding: 8px 12px; border-radius: 6px; border: 1px solid #5d4037; }";
  html += ".del-btn { background: #8b0000; color: white; border: none; border-radius: 4px; padding: 4px 8px; cursor: pointer; }";
  html += ".del-btn:hover { background: #ff4d4d; }";
  html += "</style></head><body><div class='container'>";
  html += "<h1 style='text-align:center; color:#ffd700; font-size:20px; margin-top:0; text-shadow: 2px 2px 4px #000;'>Janovy hodiny 🎣</h1>";
  html += "<div class='menu'><a href='/'>Nákup</a><a href='/screensaver'>Spořič</a><a href='/melodies'>Melodie</a></div>";
  html += "<div class='cat-track'><div class='walking-cat'>🐈</div></div>";
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
  html += "<form action='/add' method='POST' class='input-group'><input type='text' name='item' placeholder='Nová položka...' required><button type='submit'>🐟 Zaseknout</button></form><ul>";
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
  html += "<p style='text-align: center; color: #a0c4df;'>Vyberte spořič a spusťte jej:</p>";
  
  html += "<form action='/trigger-saver' method='GET'>";
  html += "<div class='input-group'>";
  html += "<select name='mode'>";
  
  // Automatické vygenerování položek roletky
  for (int i = 0; i < numSaverItems; i++) {
    html += "<option value='" + String(saverItems[i].id) + "'>" + String(saverItems[i].name) + "</option>";
  }
  
  html += "</select>";
  html += "<button type='submit'>🐟 Spustit</button>";
  html += "</div>";
  html += "</form>";

  html += "<hr style='border:0; border-top:1px solid #5d4037; margin:20px 0;'>";
  html += "<a href='/trigger-saver?state=off' class='btn btn-block' style='background: #4a5568; border-color: #718096;'>💤 Vypnout spořič</a>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

/// 3. Ovládání melodií s roletkou
void handleMelodiesWeb() {
  String html = getHtmlHeader("Přehrát melodii");
  html += "<h2>🎵 Melodie</h2>";
  html += "<p style='text-align: center; color: #a0c4df;'>Vyberte melodii k přehrání:</p>";
  
  html += "<form action='/play' method='GET'>";
  html += "<div class='input-group'>";
  html += "<select name='song'>";
  html += "<option value='happy'>Happy</option>";
  html += "<option value='obed'>Oběd</option>";
  html += "</select>";
  html += "<button type='submit'>🐟 Přehrát</button>";
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
  html += "<button type='submit' class='btn btn-block'>🐟 Uložit a restartovat</button>";
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
    server.send(200, "text/html", "<body style='background:#0f2027;color:#fff;font-family:Georgia;text-align:center;padding-top:50px;'><h3>WiFi uložena! Probíhá restart zařízení...</h3></body>");
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/html", "Chybí SSID!");
  }
}

#endif
