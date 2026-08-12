#ifndef OBED_H
#define OBED_H

#include "Buzzer.h"

const Note obed[] PROGMEM = {
  NOTE_C5, 4, 
  NOTE_E5, 4, 
  NOTE_G5, 4, 
  NOTE_C6, 2,
  NOTE_G5, 4, 
  NOTE_C6, 2
};

const uint16_t OBED_LENGTH = sizeof(obed) / sizeof(obed[0]);

#endif