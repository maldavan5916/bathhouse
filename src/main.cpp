#include "pitches.h"
#include <Arduino.h>
#include <Wire.h>
#include <LCD_1602_RUS.h>
#include <GyverDS18.h>

//============================================Setings===================================================
LCD_1602_RUS lcd(0x27, 16, 2);               // адрес дисплея и его размерность (16x2)
GyverDS18Single sensor_HW(2);                // датчик температуры горячей воды (D2)
GyverDS18Single sensor_CW(9);                // датчик температуры холодной воды (D9)
GyverDS18Single sensor_PRN(7);               // датчик температуры парной (D7)
const int Button = 5;                        // Номер пина подключения кнопки
const int Heater = 12;                       // Номер выхода нагреватель счётчика
const int Alarm = 13;                        // Номер выхода Alarm
const int heaterHW = 10;                     // номер пина к кторому подключен нагреватель
const int interval = 1000;                   // Интервал опраса датчиков времени в миллисекундах (1 секунда)
const unsigned long intervalCheck = 300000;  // Интервал проверки тепературы счётчика в милисекундах (5 мин)
const int extremelyLowTemp = 3;              // предельно низкая температура счётчика
const int MeterTemperature = 10;             // тепература до которой будет прогрет счетчик
const int longBtnClickTime = 300;            // время длинного нажатия кнопки милисекунд
const int HeatingHWTime = 60;                // время подогрева горячей воды в минутах
const int extremelyMaxTemp = 70;             // максимальная температура горячей воды
const int melody[] = {NOTE_A7, NOTE_G7, NOTE_E7, NOTE_C7, NOTE_D7, NOTE_B7, NOTE_F7, NOTE_C7}; // мелоди при успешном включении
//============================================Setings===================================================

int nScreen = 0;
String lastScreen;
unsigned long previousMillis = 0;
unsigned long lastCheck = 0;
unsigned long lastDecrementHW = 0;
int currentHeatingHWTime = 0;
bool IsHeatingMeter = false;

void successfulInclusion();
void btnClickSound();
void btnClick();
void updateLCD(String, String);
String getTemp(GyverDS18Single);
int getTempInt(GyverDS18Single);
void SetHeatingHW();

void setup()
{
  Serial.begin(115200);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(5, 0);
  lcd.print("Привет");
  
  delay(500);
  
  pinMode(Button, INPUT);   // Инициализируем пин, подключенный к кнопке, как вход (кнопка)
  pinMode(Heater, OUTPUT);  // Инициализируем пин, подключенный к светодиоду, как выход (нагреватель счётчика)
  pinMode(Alarm, OUTPUT);   // Инициализируем пин, подключенный к светодиоду, как выход (Alarm)
  pinMode(heaterHW, OUTPUT);
  digitalWrite(Alarm, LOW); // выключаем Alarm

  Serial.println("===START===");
  
  successfulInclusion(); // звук "Успешное включение"
  
  lcd.clear();
}

void loop()
{
  if (digitalRead(Button) == 1) btnClick();
  
  switch (nScreen)
  {
    case 0: {
      String addStr = currentHeatingHWTime > 0 ? "         " + String(currentHeatingHWTime) : "";
      updateLCD("Горячая вода", getTemp(sensor_HW) + addStr); 
    } break;
    case 1: { 
      String addStr = IsHeatingMeter ? "   подогрев" : "";
      updateLCD("Холодная вода", getTemp(sensor_CW) + addStr); 
    } break;
    case 2: { updateLCD("Темп. парной",     getTemp(sensor_PRN)); } break;
    case 3: { updateLCD("ПРОГРЕВ СЧЁТЧИКА", ""); } break;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    Serial.println(
      " HW: " + getTemp(sensor_HW) +
      " CW: " + getTemp(sensor_CW) +
      " PRN: " +  getTemp(sensor_PRN)
    );
  }

  if (currentMillis - lastCheck >= intervalCheck) {
    lastCheck = currentMillis;
    if (getTempInt(sensor_CW) < extremelyLowTemp) {
      nScreen = 3;
      digitalWrite(Heater, HIGH);  // включаем нагреватель
      IsHeatingMeter = true;
      
      digitalWrite(Alarm, HIGH);  // включаем сигнал
      delay(50);
      digitalWrite(Alarm, LOW);  // выключаем сигнал
      
      Serial.println("\nWARMING THE COUNTER ON\n");
    }
    else if (getTempInt(sensor_CW) >= MeterTemperature){
      digitalWrite(Heater, LOW);  // выключаем нагреватель
      IsHeatingMeter = false;
    }
  }

  if (currentMillis - lastDecrementHW >= 60000 && currentHeatingHWTime > 0) {
    currentHeatingHWTime--;
    lastDecrementHW = currentMillis;
    if (currentHeatingHWTime == 0 || getTempInt(sensor_HW) >= extremelyMaxTemp) digitalWrite(heaterHW, LOW);
  }  
}

void successfulInclusion()
{
  for (int i = 0; i < 8; i++)
  {
    tone(Alarm, melody[i]);
    delay(100);
  }
  noTone(Alarm);
}

void btnClick() {
  btnClickSound();

  unsigned long start = millis();
  bool isLongClik = false;
  while (digitalRead(Button) == 1)
  {
    if (millis() - start == longBtnClickTime) {
      btnClickSound();
      Serial.println("\nButton long clicked\n");
      isLongClik = true;
    }
  }
  if (isLongClik) {
    SetHeatingHW();  // действие при долгой нажатии на кнопку
    return;
  };
  
  nScreen++;
  if (nScreen >= 3) nScreen = 0;
  delay(250);

  Serial.println("\nButton clicked\n");
}

void btnClickSound()
{
  digitalWrite(Alarm, HIGH);  // включаем сигнал
  delay(50);
  digitalWrite(Alarm, LOW);  // выключаем сигнал
}

void updateLCD(String str1, String str2) {
  if (lastScreen == str1 + str2) return; else lastScreen = str1 + str2;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(str1);
  lcd.setCursor(0, 1);
  lcd.print(str2);
  
  Serial.println("\nLCD updated\n");
}

String getTemp(GyverDS18Single sensor) {
  sensor.requestTemp();
    if (sensor.readTemp()) {
      return String(sensor.getTempInt()) + "°C";
    }
    else {
      return "Ошибка!";
    }
}

int getTempInt(GyverDS18Single sensor) {
  sensor.requestTemp();
    if (sensor.readTemp()) {
      return sensor.getTempInt();
    }
    else {
      return -1;
    }
}

void SetHeatingHW() {
  if (nScreen != 0) return;

  if (currentHeatingHWTime == 0) { 
    currentHeatingHWTime = HeatingHWTime;
    digitalWrite(heaterHW, HIGH);

    Serial.println("HeatingHW ON");
  }
  else {
    currentHeatingHWTime = 0;
    digitalWrite(heaterHW, LOW);

    Serial.println("HeatingHW OFF");
  }

  lastDecrementHW = millis();
}