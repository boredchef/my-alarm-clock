#include <Encoder.h>

Encoder encoder(2, 3);

const int buzzer = 8;

const int buttons[] = {5, 6, 7};
int buttonState[3];
int lastButtonState[3] = {HIGH, HIGH, HIGH};

unsigned long lastDebounceTime[3] = {0, 0, 0};
unsigned long debounceDelay = 50;

// clock
bool alarmOn = false;
bool snooze = false;

enum ClockState { NORMAL,
                  SET_TIME,
                  SET_ALARM,
                  ALARM_RINGING
};
ClockState currentState = NORMAL;
enum FieldState { HOURS,
                  MINUTES,
                  AM_PM
};
FieldState fieldState = HOURS;

int currentHour = 12, currentMinute = 00, currentSec = 50;
bool currentIsPM = false;

int alarmHour = 12, alarmMin = 1;
bool alarmIsPM = false;
bool alarmEnabled = false;

unsigned long lastTick = 0;

void setup() {
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  pinMode(buttons[0], INPUT_PULLUP);
  pinMode(buttons[1], INPUT_PULLUP);
  pinMode(buttons[2], INPUT_PULLUP);
}

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - lastTick >= 1000) {
    lastTick = currentMillis;

    currentSec++;
    if (currentSec >= 60) {
      currentSec = 0;
      currentMinute++;
      if (currentMinute >= 60) {
        currentMinute = 0;
        currentHour++;
        if (currentHour == 12) currentIsPM = !currentIsPM;
        if (currentHour > 12) currentHour = 1;
      }
    }
    Serial.print(currentHour);
    Serial.print(":");
    Serial.print(currentMinute);
    Serial.print(":");
    Serial.print(currentSec);
    Serial.print(" ");
    if(currentIsPM) {
      Serial.println("PM");
    } else {
      Serial.println("AM");
    }
  }

  checkButtons();
  setTimeAndAlarm();
  checkAlarm();
}

int buttonCheck(int index) {
  int reading = digitalRead(buttons[index]);
  if(reading != lastButtonState[index]) {
    lastDebounceTime[index] = millis();
  }

  if((millis() - lastDebounceTime[index]) > debounceDelay) {
    if(reading != buttonState[index]) {
      buttonState[index] = reading;

      if(buttonState[index] == LOW) {
        lastButtonState[index] = reading;
        return 1;
      }
    }

  }
  lastButtonState[index] = reading;
  return 0;
}

void checkAlarm() {
  if(alarmHour == currentHour && alarmMin == currentMinute && alarmIsPM == currentIsPM && alarmEnabled == true) {
    currentState = ALARM_RINGING;
    Serial.println("Your alarm is ringing!");
    tone(buzzer, 1000);
  }
}

void setTimeAndAlarm() {
  long delta = encoder.readAndReset();
  if(delta == 0) return;
  
  switch(currentState) {
    case SET_TIME:
      switch(fieldState) {
        case HOURS:
          currentHour += delta;
          while(currentHour > 12) currentHour -= 12;
          while(currentHour < 1) currentHour += 12;
          break;
        case MINUTES:
          currentMinute += delta;
          if(currentMinute > 59) currentMinute = 0;
          if(currentMinute < 0) currentMinute = 59;
          break;
        case AM_PM:
          currentIsPM = !currentIsPM;
          break;
      }
    break;
    case SET_ALARM:
      switch(fieldState) {
        case HOURS:
          alarmHour += delta;
          if(alarmHour > 12) alarmHour = 1;
          if(alarmHour < 1) alarmHour = 12;
          break;
        case MINUTES:
          alarmMin += delta;
          if(alarmMin > 59) alarmMin = 0;
          if(alarmMin < 0) alarmMin = 59;
          break;
        case AM_PM:
          alarmIsPM = !alarmIsPM;
          break;
      }
  }

}

void checkButtons() {
  if(buttonCheck(0)) {
    if(currentState == NORMAL) {
      currentState = SET_TIME;
      Serial.println("State: SET_TIME");
    } else if (currentState == SET_TIME) {
      currentState = SET_ALARM;
      Serial.println("State: SET_ALARM");
    } else {
      currentState = NORMAL;
      Serial.println("State: NORMAL");
    }
  }

  switch(currentState) {
    case NORMAL:
      if(buttonCheck(2)) {
        alarmEnabled = !alarmEnabled;
        Serial.println(alarmEnabled ? "Alarm is enabled" : "Alarm is disabled");
      }
      break;
    case SET_TIME:
      if(buttonCheck(1)) {
        if(fieldState == HOURS) {
          fieldState = MINUTES;
        } else if (fieldState == MINUTES) {
          fieldState = AM_PM;
        } else {
          fieldState = HOURS;
        }
      }
      break;
    case SET_ALARM:
      if(buttonCheck(1)) {
        if(fieldState == HOURS) {
          fieldState = MINUTES;
        } else if (fieldState == MINUTES) {
          fieldState = AM_PM;
        } else {
          fieldState = HOURS;
        }
      }
      break;
    case ALARM_RINGING:
      if(buttonCheck(2)) {
        currentState = NORMAL;
        noTone(buzzer);
        Serial.println("You're now awake.");
        alarmEnabled = false;
      }
      break;
    default: 
      break;
  }
}


