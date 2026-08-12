#include "Buzzer.h"

Buzzer::Buzzer(uint8_t pin) {
  _pin = pin;
}

void Buzzer::begin() {
  ledcAttach(_pin, 2000, 8);
}

void Buzzer::playTone(int frequency, int duration) {
  if (frequency > 0) {
    ledcWriteTone(_pin, frequency);
  } else {
    ledcWriteTone(_pin, 0); // Pauza
  }
  delay(duration);
  ledcWriteTone(_pin, 0);
  delay(20); // Krátká mezera mezi tóny
}

void Buzzer::playMelody(const Note* melody, int length, int tempo) {
  for (int i = 0; i < length; i++) {
    int divider = melody[i].duration;
    int noteDuration = 0;

    if (divider > 0) {
      // Normální nota
      noteDuration = (60000 * 4) / (tempo * divider);
    } else if (divider < 0) {
      // Tečkovaná nota (záporná hodnota -> vezmeme absolutní hodnotu a zvětšíme 1.5x)
      noteDuration = (60000 * 4) / (tempo * -divider);
      noteDuration = (int)(noteDuration * 1.5);
    }

    playTone(melody[i].frequency, noteDuration);
  }
}