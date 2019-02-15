#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

bool debug = false;

const int analog_num = 0;
const int digital_num = 8;
const int analog_max = 5;
const int digital_max = 11;
const int channel_num = 5;
const int log_num = 100;

const int ledPin = 13;
const int channelPin = A0;

const int digital_pin[digital_max] = {
  2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
};
const int analog_pin[analog_max] = {
  A1, A2, A3, A4, A5
};

int digital_note[digital_max] = {
  60, 62, 64, 65, 67, 69, 71, 72, 74, 76, 77
};

int analog_control[analog_max] = {
  1, 2, 3, 4, 5
};

int velo = 127;

struct tLog
{
  bool digital[digital_num];
  int analog[analog_num];
};

int temp = 0;
tLog saved_log[log_num];
tLog last_statue, new_statue;

bool turnning[analog_max] = {0};
bool lighting = false;
bool sending = false;
int cur_channel = 0;
int cur_log = -1;

void blinkLed(bool light) {
  static int time = 0;
  if (light) {
    time = 100;
  }
  if (time > 0) {
    time--;
  }
  
  if (time > 0) {
    if (!lighting) {
      digitalWrite(ledPin, HIGH);
      lighting = true;
    }
  }
  else {
    if (lighting) {
      digitalWrite(ledPin, LOW);
      lighting = false;
    }
  }
}

void readChannel() {
  temp = analogRead(channelPin);
  if (temp <= 196) {
    cur_channel = 1;
  }
  else if (temp >= 206 && temp <= 403) {
    cur_channel = 2;
  }
  else if (temp >= 413 && temp <= 610) {
    cur_channel = 3;
  }
  else if (temp >= 620 && temp <= 817) {
    cur_channel = 4;
  }
  else if (temp >=827) {
    cur_channel = 5;
  }
}

void readData() {
  for (int i = 0; i < digital_num; i++) {
    saved_log[cur_log].digital[i] = digitalRead(digital_pin[i]);
  }
  for (int i = 0; i < analog_num; i++) {
    saved_log[cur_log].analog[i] = map(analogRead(analog_pin[i]), 0, 1023, 0, 127);
  }
}

void digitalAnalyse() {
  for (int i = 0; i < digital_num; i++) {
    if (!last_statue.digital[i]) {
      if (saved_log[cur_log].digital[i]) {
        new_statue.digital[i] = true;
      }
      else {
        new_statue.digital[i] = false;
      }
    }
    else {
      if (saved_log[cur_log].digital[i]) {
        new_statue.digital[i] = true;
      }
      else {
        temp = 0;
        for (int j = 0; j < log_num; j++) {
          if (saved_log[j].digital[i]) {
            temp = 1;
            break;
          }
        }
        if (temp) {
          new_statue.digital[i] = true;
        }
        else {
          new_statue.digital[i] = false;
        }
      }
    }
  }
}

void analogAnalyse() {
  for (int i = 0; i < analog_num; i++) {
    new_statue.analog[i] = 3 * (int)(saved_log[cur_log].analog[i] / 3);
  }
}

void analyse() {
  digitalAnalyse();
  analogAnalyse();
}

void sendData() {
  for (int i = 0; i < digital_num; i++) {
    if (new_statue.digital[i] != last_statue.digital[i]) {
      if (new_statue.digital[i]) {
        if (!debug) {
          MIDI.sendNoteOn(digital_note[i] + (cur_channel - 3) * 12, velo, cur_channel);
        }
        else {
          Serial.println("Note On");
          Serial.print(digital_note[i] + (cur_channel - 3) * 12);
          Serial.print(" ");
          Serial.print(velo);
          Serial.print(" ");
          Serial.println(cur_channel);
          Serial.println();
        }
        sending = true;
      }
      else {
        if (!debug) {
          MIDI.sendNoteOff(digital_note[i] + (cur_channel - 3) * 12, 0, cur_channel);
        }
        else {
          Serial.println("Note Off");
          Serial.print(digital_note[i] + (cur_channel - 3) * 12);
          Serial.print(" ");
          Serial.print(velo);
          Serial.print(" ");
          Serial.println(cur_channel);
          Serial.println();
        }
        sending = true;
      }
    }
  }

  for(int i = 0; i < analog_num; i++) {
    if (new_statue.analog[i] != last_statue.analog[i]) {
      if (!debug) {
        MIDI.sendControlChange(analog_control[i], new_statue.analog[i], cur_channel);
      }
      else {
        Serial.println("Control Change");
        Serial.print(analog_control[i]);
        Serial.print(" ");
        Serial.print(new_statue.analog[i]);
        Serial.print(" ");
        Serial.println(cur_channel);
        Serial.println();
      }
      sending = true;
    }
  }
}

void setup() {
  if (!debug) {
    MIDI.begin(1);
  }
  else {
    Serial.begin(9600);
  }
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(channelPin, INPUT);

  for (int i = 0; i < digital_num; i++) {
    pinMode(digital_pin[i], INPUT);
  }
  for (int i = 0; i < analog_num; i++) {
    pinMode(analog_pin[i], INPUT);
  }
  
  for (int i = 0; i < log_num; i++)
  {
    for (int j = 0; j < digital_num; j++) {
      saved_log[i].digital[j] = false;
    }
    for (int j = 0; j < analog_num; j++) {
      saved_log[i].analog[j] = 0;
    }
  }

  for (int i = 0; i < digital_num; i++) {
    last_statue.digital[i] = false;
    new_statue.digital[i] = false;
  }
  for (int i = 0; i < analog_num; i++) {
    last_statue.analog[i] = 0;
    new_statue.analog[i] = 0;
  }
}

void loop() {
  sending = false;
  
  cur_log++;
  cur_log %= log_num;

  readChannel();
  readData();
  analyse();
  sendData();

  blinkLed(sending);
  
  last_statue = new_statue;
}
