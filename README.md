# ESP32 Multifunkční Displej

Tento projekt využívá mikrokontrolér **ESP32** a **LCD displej (ST7920)** k vytvoření chytrého domácího asistenta, který zobrazuje aktuální informace, počasí a umožňuje snadnou interakci přes webové rozhraní.

## Klíčové funkce
*   **Počasí:** Automatické zjišťování polohy a stahování dat o počasí.
*   **Čas:** Synchronizace času přes NTP server.
*   **Kurzy měn:** Získávání aktuálních kurzů z ČNB (EUR, USD).
*   **Slunce:** Zobrazení časů východu a západu slunce.
*   **Nákupní seznam:** Správa nákupního seznamu přímo na displeji i přes web.
*   **Audio signalizace:** Přehrávání melodií (např. upozornění na oběd) pomocí bzučáku.
*   **Spořič obrazovky:** Různé animace (kočka, ryba, hvězdy, vlastní bitmapy).
*   **Webové rozhraní:** Možnost ovládání, správy nákupního seznamu a nastavení WiFi přes vestavěný web server.
*   **Uživatelské rozhraní:** Ovládání pomocí enkodéru (otočný volič + tlačítko).

## Hardwarové požadavky
*   ESP32 DevKit
*   LCD displej ST7920 (128x64)
*   Rotační enkodér (pin A, pin B, tlačítko)
*   Bzučák (pasivní)
*   WiFi připojení

## Knihovny
Projekt vyžaduje následující Arduino knihovny:
*   `U8g2` (pro ovládání displeje)
*   `WiFi.h` (součást ESP32 core)
*   `WebServer.h` (součást ESP32 core)
*   `HTTPClient.h` (součást ESP32 core)
*   `Buzzer.h` (vlastní nebo kompatibilní knihovna)

## Nastavení a konfigurace
1.  **WiFi:** Zařízení se při prvním spuštění pokusí připojit k přednastavené síti. Pokud selže, aktivuje AP režim `ESP32-Nastaveni` (192.168.4.1) pro konfiguraci.
2.  **Webové rozhraní:** Po připojení k WiFi můžete k zařízení přistoupit přes prohlížeč zadáním IP adresy zařízení v síti.
3.  **Ovládání:** 
    *   **Otočení:** Pohyb v menu / seznamu.
    *   **Stisk:** Potvrzení výběru / návrat z podmenu.

## Struktura projektu
*   `main.ino`: Hlavní kód logiky.
*   `html.h`: Webové rozhraní a obsluha požadavků.
*   `*.h`: Soubory s animacemi, fonty a melodiemi.

---
*Projekt vytvořen pro ESP32 s využitím standardního Arduino frameworku.*

License
This project is licensed under the MIT License - see the LICENSE file for details.
