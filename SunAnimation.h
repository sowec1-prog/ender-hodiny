#ifndef SUN_ANIMATION_H
#define SUN_ANIMATION_H

#include <Arduino.h>
#include <U8g2lib.h>

void drawSunrise(U8G2 &u8g2, unsigned long startTime, String timeStr) {
  unsigned long duration = 300000; // 5 minut v milisekundách
  unsigned long elapsed = millis() - startTime;
  
  if (elapsed > duration) elapsed = duration;

  int sunPosY = 60 - (int)((elapsed * (60 - 30)) / duration);

  u8g2.drawHLine(0, 52, 128);
  u8g2.drawBox(0, 53, 128, 11); 

  int centerX = 64; 
  int r = 12;

  u8g2.drawDisc(centerX, sunPosY, r);

  if (sunPosY < 50) {
    u8g2.drawLine(centerX, sunPosY - (r+6), centerX, sunPosY - (r+2)); 
    u8g2.drawLine(centerX, sunPosY + (r+6), centerX, sunPosY + (r+2)); 
    u8g2.drawLine(centerX - (r+6), sunPosY, centerX - (r+2), sunPosY); 
    u8g2.drawLine(centerX + (r+6), sunPosY, centerX + (r+2), sunPosY); 
    
    u8g2.drawLine(centerX - 10, sunPosY - 10, centerX - 7, sunPosY - 7);
    u8g2.drawLine(centerX + 10, sunPosY - 10, centerX + 7, sunPosY - 7);
    u8g2.drawLine(centerX - 10, sunPosY + 10, centerX - 7, sunPosY + 7);
    u8g2.drawLine(centerX + 10, sunPosY + 10, centerX + 7, sunPosY + 7);
  }

  // Text s časem a pozdravem dole
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_6x12_tr);
  
  String sunText = timeStr + " DOBRE RANO";
  // Vycentrování textu na šířku 128 px (přibližný odhad délky řetězce)
  u8g2.drawStr(12, 61, sunText.c_str());
  
  u8g2.setDrawColor(1);
}

#endif