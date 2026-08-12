#ifndef FISH_ANIMATION_H
#define FISH_ANIMATION_H

#include <Arduino.h>
#include <U8g2lib.h>

// Funkce pro vykreslení detailní štiky, plynulých vlnovek a času plovoucího s rybou
void drawPike(U8G2 &u8g2, int posX, String timeStr) {
  int y = 32; // Střed obrazovky

  // Čas plovoucí s rybou (přesně nad jejím tělem)
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(posX + 16, y - 9, timeStr.c_str());

  // 1. Hlavní tělo štiky (dlouhé štíhlé torpédo)
  u8g2.drawBox(posX + 14, y - 5, 46, 10);
  
  // 2. Hlava a specifická protáhlá tlama (směřuje doprava)
  u8g2.drawTriangle(posX + 60, y - 5, posX + 60, y + 5, posX + 78, y); 
  
  // 3. Oko štiky
  u8g2.setDrawColor(0); 
  u8g2.drawPixel(posX + 65, y - 2);
  u8g2.setDrawColor(1);

  // 4. Ocasní ploutev (vlevo)
  u8g2.drawTriangle(posX + 14, y, posX + 3, y - 11, posX + 8, y - 2);
  u8g2.drawTriangle(posX + 14, y, posX + 3, y + 11, posX + 8, y + 2);
  u8g2.drawLine(posX + 8, y - 2, posX + 8, y + 2); 

  // 5. Hřbetní a břišní ploutve
  u8g2.drawTriangle(posX + 22, y - 5, posX + 27, y - 11, posX + 33, y - 5); 
  u8g2.drawTriangle(posX + 25, y + 5, posX + 30, y + 10, posX + 36, y + 5); 

  // 6. Vykreslení plynulých vlnovek dole
  int waveBaseY = 57;
  for (int x = -16; x < 128; x += 24) {
    int wx = x + (posX / 2) % 24; 
    
    u8g2.drawLine(wx,      waveBaseY,     wx + 6,  waveBaseY - 3); 
    u8g2.drawLine(wx + 6,  waveBaseY - 3, wx + 12, waveBaseY + 3); 
    u8g2.drawLine(wx + 12, waveBaseY + 3, wx + 18, waveBaseY);     
  }
}

#endif