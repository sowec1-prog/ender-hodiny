#ifndef STARS_ANIMATION_H
#define STARS_ANIMATION_H

#include <Arduino.h>
#include <U8g2lib.h>

// Struktura pro jednu hvězdu
struct Star {
  int x;
  int y;
  bool active;
};

// Vytvoříme pole pro 30 hvězd na obrazovce 128x64
const int NUM_STARS = 30;
Star stars[NUM_STARS];
bool starsInitialized = false;

// Inicializace náhodných pozic hvězd
void initStars() {
  for (int i = 0; i < NUM_STARS; i++) {
    stars[i].x = random(0, 128);
    stars[i].y = random(0, 50); // Necháme místo dole pro Měsíc nebo text
    stars[i].active = random(0, 2); // Náhodně zapnuto/vypnuto
  }
  starsInitialized = true;
}

// Funkce pro vykreslení hvězd, měsíce a času
void drawStars(U8G2 &u8g2, String timeStr) {
  if (!starsInitialized) {
    initStars();
  }

  // Občasně náhodně přepneme stav některých hvězd, aby blikaly
  for (int i = 0; i < NUM_STARS; i++) {
    if (random(0, 10) > 6) { 
      stars[i].active = !stars[i].active;
    }
    
    // Vykreslení aktivní hvězdy
    if (stars[i].active) {
      if (i % 5 == 0) {
        // Větší zářící hvězda (křížek)
        int x = stars[i].x;
        int y = stars[i].y;
        u8g2.drawPixel(x, y);
        u8g2.drawPixel(x - 1, y);
        u8g2.drawPixel(x + 1, y);
        u8g2.drawPixel(x, y - 1);
        u8g2.drawPixel(x, y + 1);
      } else {
        // Normální malá hvězda (tečka)
        u8g2.drawPixel(stars[i].x, stars[i].y);
      }
    }
  }

  // Nakreslíme stylizovaný Měsíc v rohu (z původního kódu)[cite: 5]
  u8g2.drawDisc(110, 15, 10);
  u8g2.setDrawColor(0); // Vyříznutí srpku měsíce[cite: 5]
  u8g2.drawDisc(106, 12, 9);
  u8g2.setDrawColor(1);

  // Vykreslení aktuálního času na noční oblohu
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setCursor(48, 10);
  u8g2.print(timeStr);
}

#endif